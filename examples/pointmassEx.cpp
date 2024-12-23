#include "ALILQGames/ALILQGames.h"
#include "ALILQGames/pointmass.h"
#include "ALILQGames/NPlayerModel.h"
#include "ALILQGames/BoxConstraint.h"
#include "ALILQGames/CollisionConstraint2D.h"
#include "ALILQGames/CollisionCost2D.h"
#include "ALILQGames/costDiffDrive.h"
#include "ALILQGames/AL.h"
#include "ALILQGames/SolverParams.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <typeinfo>

using namespace std;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

// void* operator new(size_t size)
// {
//     std::cout << "Allocating " << size << " bytes\n";
//     return malloc(size);
// }

int main(){

    // Player 1 costs
    MatrixXd Q1 = MatrixXd::Zero(8,8);
    Q1.block(0,0,4,4) = 0.0*MatrixXd::Identity(4,4);

    MatrixXd QN1 = MatrixXd::Zero(8,8);
    QN1.block(0,0,4,4) = 30.0*MatrixXd::Identity(4,4);
    
    MatrixXd R1 = MatrixXd::Zero(4,4);
    R1.block(0,0,2,2) = 2.0*MatrixXd::Identity(2,2);

    // Player 2 costs
    MatrixXd Q2 = MatrixXd::Zero(8,8);
    Q2.block(4,4,4,4) = 0.1*MatrixXd::Identity(4,4);

    MatrixXd QN2 = MatrixXd::Zero(8,8);
    QN2.block(4,4,4,4) = 30.0*MatrixXd::Identity(4,4);
    
    MatrixXd R2 = MatrixXd::Zero(4,4);
    R2.block(2,2,2,2) = 2.0*MatrixXd::Identity(2,2);

    // Initialize a 2 player point mass
    VectorXd x0(8);

    VectorXd xgoal(8);
    VectorXd u(4);

    x0 << 0.0, 1.5, 0.0, 0.0, 1.5, 0.0, 0.0, 0.0;
    xgoal << 5.0, 1.5, 0.0, 0.0, 1.5, 5.0, 0.0, 0.0;

    u << 0.0, 0.0, 0.0, 0.0;

    double dt = 0.1;

    int nx = 4;
    int nu = 2;
    int n_agents = 2;

    SolverParams params;
    params.nx = nx;
    params.nu = nu;
    params.n_agents = n_agents;
    params.Nx = nx*n_agents;
    params.Nu = nu*n_agents;

    params.p_inq = 2*(params.Nx + params.Nu) + n_agents*(n_agents - 1);   // state and input constraints + number of collisions constraints btn agents

    VectorXd umin = -5.0*VectorXd::Ones(nu*n_agents);
    VectorXd umax =  5.0*VectorXd::Ones(nu*n_agents);

    VectorXd xmin = -100.0*VectorXd::Ones(nx*n_agents);
    VectorXd xmax =  100.0*VectorXd::Ones(nx*n_agents);

    VectorXd r_avoid(n_agents);

    r_avoid(0) = 1.0;
    r_avoid(1) = 1.0;
//     r_avoid(2) = 1.0;
    // r_avoid(3) = 1.0;


    VectorXd cinq = VectorXd::Zero(n_agents*(n_agents - 1));

    VectorXd xtest(n_agents*nx);
    xtest << 1.5, 2.5, 0.0, 0.0, 
            1.2, 2.4, 0.0, 0.0;
        //     2.3, 2.1, 0.0, 0.0;
            // 1.5, 1.9, 0.0, 0.0;
    
    VectorXd utest(n_agents*nu);
    utest << 1.5, 2.5, 
            1.2, 2.4;
        //     2.3, 2.1;
            // 0.0, 0.0;

    VectorXd lx = VectorXd::Zero(n_agents*nx);
    MatrixXd lxx = MatrixXd::Zero(n_agents*nx, n_agents*nx);

    VectorXd lu = VectorXd::Zero(n_agents*nu);
    MatrixXd luu = MatrixXd::Zero(n_agents*nu, n_agents*nu);

    
    double rho = 20.0;

    // std::vector<std::shared_ptr<Cost>> pc;  
    // pc.push_back( std::shared_ptr<Cost> (new BoxConstraint(umin, umax, xmin, xmax);) );   
    // pc.push_back( std::shared_ptr<Cost> (new DiffDriveCost(Q2, QN2, R2, xgoal)) );



    std::vector<std::shared_ptr<GlobalConstraints>> ptr_constr; 
    ptr_constr.push_back( std::shared_ptr<GlobalConstraints> (new CollisionConstraint2D(params, r_avoid)) );
    ptr_constr.push_back( std::shared_ptr<GlobalConstraints> (new BoxConstraint(umin, umax, xmin, xmax)) );   


    AL* al = new AL(params, ptr_constr);

    al->ConcatConstraint(xtest, utest);
    // std::cout << "p " << params.p_inq << "\n";



    // collision->StageCostGradient(0, lx, lu, xtest, u);

    // collision->StageCostHessian(1, lxx, luu, xtest, u);

    // std::cout << "lxx \n" << lxx << "\n";



    // auto t0 = high_resolution_clock::now();
    // cg -> StateAndInputConstraint(cinq ,xtest ,utest);
    // auto t1 = high_resolution_clock::now();

    // /* Getting number of milliseconds as a double. */
    // duration<double, std::milli> ms_double = t1 - t0;
    // std::cout << ms_double.count() << "ms\n";

    // MatrixXd cx = MatrixXd::Zero(2*(nx + nu), nx);
    // cg->StateConstraintJacob(cx, xtest);
    // std::cout << "\n" << cx << "\n";
    // // Model& PointMass(dt);
    // Model* pm = new PointMass(dt);                    // heap allocation

    // NPlayerModel* Npm = new NPlayerModel(pm, 2);

    // std::vector<std::shared_ptr<Cost>> pc;  
    // pc.push_back( std::shared_ptr<Cost> (new DiffDriveCost(Q1, QN1, R1, xgoal)) );   
    // pc.push_back( std::shared_ptr<Cost> (new DiffDriveCost(Q2, QN2, R2, xgoal)) );  



    return 0;
}