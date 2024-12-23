#include "ALILQGames/ALILQGames.h"
#include "ALILQGames/Timer.h"

double ALILQGames::initial_rollout(const VectorXd& x0)
{
    x_k[0] = x0;                                // x0 
    total_cost = 0.0;

    for(size_t k=0; k < H-1; k++)                  
    {
        //u_t[k] = - K_t[k]*x_t[k] - k_t[k];                  // This is not neccasry (just an initial random rollout)
        x_k[k+1] = Nmodel->RK4(x_k[k], u_k[k] , dt);         // rollout nonlinear dynmaics with policy

    }

    return total_cost;
}

// TODO: implement mpc
double ALILQGames::forward_rollout(const VectorXd& x0)
{
    // x_hat = x_k;                                                 // previous states
    // u_hat = u_k;                                                 // previous controls

    x_k[0] = x0;                                                 // Inital State (will change for MPC) 
    max_grad = d_k[0].lpNorm<Eigen::Infinity>();                 // Want to store infinity norm of feedforward term (for convergence)

    double cost = 0.0;

    double norm_grad;

    for(size_t k=0; k < H-1; k++)                  
    {
        u_k[k] = u_hat[k] - K_k[k]*(x_k[k] - x_hat[k]) - alpha*d_k[k];        // optimal affine policy
        // std::cout << "xhat " << x_hat[k] << "\n";
        // std::cout << "x_t " << x_t[k] << "\n";
        x_k[k+1] = Nmodel->RK4(x_k[k], u_k[k] , dt);                      // rollout nonlinear dynmaics with policy

        // #pragma omp parallel for
        for (size_t i=0; i < n_agents; i++)
        {
            cost += getStageCost(i, k);
        }

        // Store Maximum gradient (to check for convergence)
        norm_grad = d_k[k].lpNorm<Eigen::Infinity>();
        if (norm_grad > max_grad)
        {
            max_grad = norm_grad;
        }
    }

    for (size_t i=0; i < n_agents; i++)
    {
        cost += getTerminalCost(i);
    }

    return cost;
}

double ALILQGames::backward_pass(const int k_now)
{
    double cost = 0.0;

    // Terminal State cost to go
    for (size_t i=0; i < n_agents; i++)            // For each agent
    {

        pc[i]->TerminalCostGradient(i, lx[i], x_k[H-1]);
        pc[i]->TerminalCostHessian(i, lxx[i], x_k[H-1]);

        P[i] = lxx[i];
        p[i] = lx[i];
        DeltaV[i] = 0.0;
        cost += getTerminalCost(i);
    }                 


    for(int k=H-2; k>=0; k--)     
    {
        Nmodel->dynamicsJacobConcat(fx, fu, x_k[k], u_k[k]);  // get the concatenated dynamics

        for (size_t i=0; i < n_agents; i++)            // For each agent
        {
            const int inu = i*nu;


            pc[i]->StageCostGradient(i, lx[i], lu[i], x_k[k], u_k[k]);
            pc[i]->StageCostHessian(i, lxx[i], luu[i], x_k[k], u_k[k]);

            al->ALGradHess(k, Lxx[i], Luu[i], Lux[i], Lx[i], Lu[i], 
                              lxx[i], luu[i], lux[i], lx[i], lu[i], 
                                                    x_k[k], u_k[k]); 

            // S = [(R¹¹ + B¹ᵀ P¹ B¹)     (B¹ᵀ P¹ B²)     ⋅⋅⋅      (B¹ᵀ P¹ Bᴺ)   ;
            //         (B²ᵀ P² B¹)     (R²² + B²ᵀ P² B²)  ⋅⋅⋅      (B²ᵀ P² Bᴺ)   ;
            //              ⋅                  ⋅        ⋅                ⋅       ;
            //              ⋅                  ⋅           ⋅             ⋅       ;
            //              ⋅                  ⋅              ⋅          ⋅       ;
            //         (Bᴺᵀ Pᴺ B¹)        (Bᴺᵀ Pᴺ B²)     ⋅⋅⋅   (Rᴺᴺ + Bᴺᵀ Pᴺ Bᴺ)], Nu × Nu

            // This naively fills the entire row       
            S.block(inu, 0, nu, Nu) 
                = fu.middleCols(inu, nu).transpose() * P[i] * fu;
            
            // The diagonals are overritten here
            S.block(inu, inu, nu, nu) 
                = Luu[i].block(inu, inu, nu, nu)
                + (fu.middleCols(inu, nu).transpose() * P[i] * fu.middleCols(inu, nu));

            // Check the Lux 
            YK.middleRows(inu, nu) 
                = fu.middleCols(inu, nu).transpose() * P[i] * fx + Lux[i].middleRows(inu, nu);
            
            Yd.segment(inu, nu) = (fu.middleCols(inu, nu).transpose() * p[i]) + Lu[i].segment(inu, nu);

        }

        Eigen::LLT<MatrixXd> lltOfS(S); // compute the Cholesky decomposition of S

        if (lltOfS.info() == Eigen::NumericalIssue)
        {
            // NotPD = true;
            cout << "S is not PD\n " <<  "\n";
        }

    
        // Do a least squares like \ in matlab??
        S = S.inverse();//lltOfS.solve(MatrixXd::Identity(Nu,Nu));
        K_k[k] = S*YK; 
        d_k[k] = S*Yd;

        F_k = fx - fu * K_k[k];
        beta_k = - fu * d_k[k];                     // n x m x m x 1 = n x 1
        
        for (size_t i=0; i < n_agents; i++)            // For each agent
        {

            // Update recursive variables P, p, and delta v

            // DeltaV seems to blow up, check for this
            // DeltaV[i] = 0.5 * ((d_k[k].transpose() * Luu[i] - 2 * Lu[i].transpose()) * d_k[k]
            // - ((P[i] * beta_k + 2 * p[i]).transpose() * beta_k).sum()) + DeltaV[i];

            // std::cout << "V[i] " << DeltaV[i] << "\n";

            p[i] = Lx[i] + (K_k[k].transpose() * (Luu[i] * d_k[k] - Lu[i])) 
                        + F_k.transpose() * (p[i] + P[i] * beta_k) - 2*Lux[i].transpose()*d_k[i];

            // P[i] = Lxx[i] + (K_k[k].transpose() * Luu[i] * K_k[k]) - Lux[i].transpose()*K_k[k] + F_k.transpose() * P[i] * F_k;

            P[i] = Lxx[i] + K_k[k].transpose() * (Luu[i] * K_k[k] - Lux[i]) + F_k.transpose() * P[i] * F_k;

            cost += getStageCost(i, k);

        }
    }
    return cost;

}

void ALILQGames::BackTrackingLineSearch(const VectorXd& x0)
{

    x_hat = x_k;                                                 // previous states
    u_hat = u_k;  
    double alphas[9] = {1.0, 0.5, 0.25, 0.125, 0.0625, 0.03125, 0.015625, 0.0078125, 0.0};
    double total_cost_now = 0.0;

    for (const double& alpha_i : alphas)//(int i=0; i<8; i++)
    {
        total_cost_now = 0.0;
        alpha = alpha_i;
        total_cost_now = forward_rollout(x0);

        // for (size_t i=0; i < n_agents; i++)            // For each agent
        // {
        //     total_cost_now += TotalCost(i);
        // }

        if (iter_ == 0)          // first iteration is random so we dont care about it (cost should increase)
        {
            break;
        }

        // if (abs(total_cost_now - total_cost) < 10000.0)
        if (total_cost_now < total_cost && abs(total_cost_now - total_cost) < 10000.0)
        {
            break;
        }

    }

    if (alpha == 0.0)
    {
        cout << "Line search scaling is very small" << "\n";
    }

    // cout << "alpha " << alpha << "\n";

    total_cost = total_cost_now;
}

void ALILQGames::ArmuijoLineSearch(const VectorXd& x0)
{
    x_hat = x_k;                                                 // previous states
    u_hat = u_k;                                                 // previous controls
    double total_cost_now = 0.0;
    double DeltaV_total = 0.0;

    forward_rollout(x0);                                       // rollout with alpha = 1
    for (size_t i=0; i < n_agents; i++)            // For each agent
    {
        // Store entire previous trajectory
        total_cost_now += pc[i]->TotalCost(i, H, x_k, u_k);
        DeltaV_total += DeltaV[i];
        // cost_now[i] = pc[i]->TotalCost(i, H, x_k, u_k);
    }
    // while (cost_now[i] > cost[i] - 1e-2*alpha*DeltaV[i])
    while (total_cost_now > total_cost - (1e-3*alpha * DeltaV_total))
    {
        // std::cout << "Hello !" << "\n";
        alpha *= 0.5;

        forward_rollout(x0);            // rollout with reduced alpha

        total_cost_now = 0.0;

        // cost_now[i] = pc[i]->TotalCost(i, H, x_k, u_k);
        for (int i=0; i < n_agents; i++)            // For each agent
        {
            total_cost_now += pc[i]->TotalCost(i, H, x_k, u_k);
        }
    }
    alpha = 1.0;                            // reset alpha
    total_cost = total_cost_now;                  // previous cost is now current cost
    // cost[i] = cost_now[i];                  // previous cost is now current cost
}

void ALILQGames::solve(SolverParams& params, const VectorXd& x0)
{
    // omp_set_num_threads(16);

    double max_violation = 0.0;
    double current_violation = 0.0;
    cout << "Solver Time " << "\n";
    {
        Timer timer;
        initial_rollout(x0);
        iter_ = 0;

        std::cout << "Hello! " << "\n";

        // Outer loop is an augmented lagrangian
        for(size_t outer_iter=0; outer_iter < params.max_iter_al; outer_iter++)
        {

            // Inner Loop is an ILQGame 
            for(size_t inner_iter=0; inner_iter < params.max_iter_ilq; inner_iter++)
            {

                total_cost = backward_pass(k_now);


                BackTrackingLineSearch(x0);

                if ( max_grad < params.grad_tol || alpha == 0.0) // If the infinity norm of gradient term is less than some tolerance, converged
                {
                    // std::cout << "Converged!" << "\n"; 
                    break;
                }

                iter_ += 1;
            }


            // Get the max violation in constraints to check if we can converge early
            max_violation = 0.0;
            current_violation = 0.0;

            for(size_t k=0; k < H-2; k++)                  // TODO: Fix indices for H-1 instead of H-2
            {
                current_violation = al -> MaxConstraintViolation(x_k[k], u_k[k]);
                if (current_violation > max_violation)
                {
                    max_violation = current_violation;
                }
            }

            // If the max violation is within some tolerance, we converged
            if ( max_violation < params.max_constraint_violation)                  
            {
                cout << "Converged!" << "\n"; 
                break;
            }

            al -> DualUpdate(x_k, u_k);

            al -> PenaltySchedule();

        }
    }

    // cout << "Solution x[end]: " << x_k[H-1] << "\n";
    // cout << "Solution Dual[end]: " << al->GetDual(H-2) << "\n";
    cout << "Maximum violation: " << max_violation << "\n";

    if (!isMPC)
    {
        al->ResetDual();
        al->ResetPenalty();
    }

    std::cout << "K Now: " << k_now << "\n";
    k_now += 1;
}

double ALILQGames::TotalCost(const int i)
{
    double current_cost = 0.0;

    // Add up stage cost
    for (size_t k=0; k < H-1; k++)
    {
        current_cost += getStageCost(i, k);
    }

    current_cost += getTerminalCost(i);
    return current_cost;
}

void ALILQGames::recedingHorizon(SolverParams& params, const VectorXd& x0)
{ 
    X_k[0] = x0;  
    const int N = params.H_all;                         // Entire horizon length
    const int Nhor = params.H;                          // MPC horizon

    // ############################### Reset all parameters (should be a function) #############################
    H = Nhor;
    k_now = 0;
    al->ResetDual();
    al->ResetPenalty();

    for (int k=0; k < N - Nhor; k++)
    {
        // std::cout << "K :" << k << "\n";

        // std::for_each(pc.begin(), pc.end(), [&k](shared_ptr<Cost>& agent_i) { 
        //     agent_i->NAgentGoalChange(k); });
        if (params.isGoalChanging)
        {
            for (int i=0; i < n_agents; i++)            // For each agent
            {
                pc[i]->BezierCurveGoal(k, N);
            }
        }

        solve(params, X_k[k]);
        X_k[k+1] = x_k[1];
        U_k[k] = u_k[0];

        u_k.erase(u_k.begin());
        u_k.push_back(u_k.back());

        if (!(k % params.reset_schedule) || (total_cost > 1000000.0))
        {
            // std::cout << "Reset Penalty\n" << (k % params.reset_schedule) << "\n";
            // std::cout << "Reset Penalty\n" << total_cost << "\n";

            al->ResetDual();
            al->ResetPenalty();
        }
    }

    for (int k = N - Nhor; k < N-1; k++)
    {
        // std::cout << "Kend :" << k << "\n";
        if (params.isGoalChanging)
        {
            for (int i=0; i < n_agents; i++)            // For each agent
            {
                pc[i]->BezierCurveGoal(k, N);
            }
        }

        solve(params, X_k[k]);
        X_k[k+1] = x_k[1];
        U_k[k] = u_k[0];

        u_k.erase(u_k.begin());                 // delete u_k[0]
        u_k.push_back(u_k.back());              // add u_k[end]

        H -= 1;

        if (!(k % params.reset_schedule) || (total_cost > 1000000.0))
        {
            al->ResetDual();
            al->ResetPenalty();
        }
    }
}

void ALILQGames::MPCWarmStart(SolverParams& params, const VectorXd& x0) 
{
    recedingHorizon(params, x0);        // run MPC
    H = params.H;                       // reset MPC Horizon
    k_now = 0;                          // reset iteration index 
    
    for (int k=0; k<H; k++)
    {
    u_k[k] = U_k[k];                    // Set warm started strategy for the first MPC Horizon
    }
}


void ALILQGames::ChangeStrategy(const int i, const float delta)
{
    for(size_t k=0; k<H-1; k++)
    {
        u_k[k].setOnes();
        u_k[k].segment(i*nu, nu) += delta*VectorXd::Ones(nu);          

        K_k[k] = 0.1*MatrixXd::Ones(Nu, Nx);                        // Feedback gain is m by n
        d_k[k] = 0.1*VectorXd::Ones(Nu);                            // Feedforward is m dimensional
    }
}


void ALILQGames::setState(const int k, const VectorXd& x)
{
    x_k[k] = x;
}


VectorXd ALILQGames::getState(const int k)
{
    return x_k[k];
}

VectorXd ALILQGames::getControl(const int k)
{
    return u_k[k];
}

VectorXd ALILQGames::getMPCState(const int k) 
{
    if (isMPC)
        return X_k[k];
    else
        cout << "Your'e not implementing MPC! Error\n";
        return x_k[k];
}

VectorXd ALILQGames::getMPCControl(const int k) 
{
    if (isMPC)
        return U_k[k];
    else
        cout << "Your'e not implementing MPC! Error\n";
        return u_k[k];
}

// Change this to augmented lagrangian cost
double ALILQGames::getStageCost(const int i, const int k)
{
    double cost = pc[i]->StageCost(i, x_k[k], u_k[k]);
    return al->Merit(k, i, cost, x_k[k], u_k[k]);
}

double ALILQGames::getTerminalCost(const int i)
{
    return pc[i]->TerminalCost(i, x_k[H-1]);
}