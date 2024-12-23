#pragma once
#include "NPlayerModel.h"
#include <iostream>
#include <vector>
#include <memory>
#include "cost.h"
#include "SolverParams.h"
#include "Solver.h"

using namespace std;

class ILQGames : public Solver
{
    public:
        // Pass a Nplayer Model system, and a vector of costs of each player

        ILQGames(SolverParams& params, NPlayerModel* ptr_model, 
                    vector<shared_ptr<Cost>> ptr_costs) : Solver(ptr_model, ptr_costs)
        {
            // Nmodel.reset(ptr_model);                 // reset shared pointer to the model 
            dt = params.dt;
            H = params.H;

            // populate terms for entire horizon
            x_k.resize(H);
            u_k.resize(H-1);
            K_k.resize(H-1);
            d_k.resize(H-1);
            lambda.resize(H);

            alpha = params.alpha;
            nx = params.nx;                                      // Number of states for agent i
            nu = params.nu;                                      // Number of controls for agent i
            Nx = params.Nx;                                      // Total number of states for all agents
            Nu = params.Nu;                                      // Total number of controls for all agents
            n_agents = params.n_agents;

            // Initialize Jacobians
            fx = MatrixXd::Zero(Nx, Nx);                            // Feedback gain is m by n
            fu = MatrixXd::Zero(Nx, Nu);                            // Feedback gain is m by n

            // Costs
            lx.resize(n_agents);
            lu.resize(n_agents);
            lxx.resize(n_agents);
            luu.resize(n_agents);
            lux.resize(n_agents);

            // Backward pass stuff 
            DeltaV.resize(n_agents);
            p.resize(n_agents);
            P.resize(n_agents);
            S = MatrixXd::Zero(Nu, Nu);                             // Same as S in the document
            F_k = MatrixXd::Zero(Nx, Nx);                           // Placeholder for eqn readibility
            beta_k = VectorXd::Zero(Nx);                         // Placeholder for eqn readibility
            YK = MatrixXd::Zero(Nu, Nx);                            // Right hand side of system of linear equations for K                              
            Yd = VectorXd::Zero(Nu);                                // Right hand side of system of linear equations for d


            for(int i=0; i < n_agents; i++)
            {
                DeltaV[i] = 0.0;
                p[i] = VectorXd::Zero(Nx);
                P[i] = MatrixXd::Zero(Nx, Nx);
                lx[i] = VectorXd::Zero(Nx);                         // gradient of cost wrt x and is m by 1
                lu[i] = VectorXd::Zero(Nu);                         // gradient of cost wrt u and is n by 1
                lxx[i] = MatrixXd::Zero(Nx, Nx);                    // Hessian of cost wrt xx and is n by n
                luu[i] = MatrixXd::Zero(Nu, Nu);                    // Hessian of cost wrt uu and is m by m
                lux[i] = MatrixXd::Zero(Nu, Nx);                    // Hessian of cost wrt ux and is m by n
            }
            

            // Fill concatenated states, controls and policies for entire horizon
            for(int k=0; k < H; k++)                                     //Maybe do std::fill                 
            {
                x_k[k] = VectorXd::Zero(Nx);                                  // State vector is n dimensional
            }

            for(int k=0; k<H-1; k++)
            {
                u_k[k] = 0.1*VectorXd::Ones(Nu);                           // Input vector is m dimensional
                // K_k[k] = 0.1*MatrixXd::Random(Nu, Nx);                        // Feedback gain is m by n
                // d_k[k] = 0.1*VectorXd::Random(Nu);                            // Feedforward is m dimensional
                K_k[k] = 0.01*MatrixXd::Ones(Nu, Nx);                        // Feedback gain is m by n
                d_k[k] = 0.01*VectorXd::Ones(Nu);                            // Feedforward is m dimensional
            }
        }

        // ILQGames();                                     // Constructor

        double initial_rollout(const VectorXd& x0) override;

        double forward_rollout(const VectorXd& x0) override;

        double backward_pass(const int k_now) override;

        void BackTrackingLineSearch(const VectorXd& x0) override;

        void ArmuijoLineSearch(const VectorXd& x0) override;

        void solve(SolverParams& params, const VectorXd& x0) override;

        // void recedingHorizon(const VectorXd& x0) override;

        // Helper functions should probably be universal (in Solver.h)

        VectorXd getState(const int k) override;

        VectorXd getControl(const int k) override;

        VectorXd getMPCState(const int k) override;

        VectorXd getMPCControl(const int k) override;
        

        double getStageCost(const int i, const int k) override;

        double getTerminalCost(const int i) override;
   

        // vector<shared_ptr<Cost>> pc;  
        // shared_ptr<NPlayerModel> Nmodel;                   // N player model; // change this


        double dt;                                   // delta t

        int H;                                      // Horizon length

        int iter_;

        double iter_cost, total_cost;

    private:

        vector<VectorXd> x_k;                  // States over the entire Horizon 

        vector<VectorXd> u_k;                  // Control inputs over the entire Horizon

        vector<VectorXd> lambda;               // Dual variable for ALL agents (here we are assuming no equality constraints)

        VectorXd c;                                 // inequality constraints

        MatrixXd cx, cu;                             // constraint jacobian

        vector<VectorXd> x_hat;                // previous state rollout
        vector<VectorXd> u_hat;                // previous controls

        vector<MatrixXd> K_k;                  // Proportional gain

        vector<VectorXd> d_k;                  // Feedforward Gain (AKA gradient)

        MatrixXd fx, fu;                            // Dynamics Jacobian

        vector<VectorXd> lx,lu;                // Cost Gradients

        vector<MatrixXd> lxx, luu, lux;        // Cost Hessians

        double max_grad;

        double alpha;

        
        int nx;                                             // Number of states for agent i
        int nu;                                     // Number of controls for agent i
        int Nx;                                     // Total number of states for all agents
        int Nu;                                     // Total number of controls for all agents
        int n_agents;

        // Backward pass stuff
        vector<MatrixXd> P;                                   // value matrix at final step is only cost/state hessian
        vector<VectorXd> p;
        vector<double> DeltaV, cost, cost_now;                  // Expected change in cost (used for linesearch)
        MatrixXd S;                                        // Same as S in the document
        MatrixXd F_k;                                      // Placeholder for eqn readibility
        VectorXd beta_k;
        MatrixXd YK;                                       // Right hand side of system of linear equations for K                              
        VectorXd Yd;                                           // Right hand side of system of linear equations for d

};