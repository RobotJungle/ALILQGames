#pragma once
#include "GlobalConstraints.h"
#include <iostream>
#include <vector>
#include <memory>
#include "SolverParams.h"

class AL
{
    public:
        
        AL(SolverParams& params, std::vector<std::shared_ptr<GlobalConstraints>> ptr_constr) : ptr_cons(move(ptr_constr))    
        {
            I_mu = MatrixXd::Zero(params.p_inq, params.p_inq);
            c = VectorXd::Zero(params.p_inq);
            cx = MatrixXd::Zero(params.p_inq, params.Nx);
            cu = MatrixXd::Zero(params.p_inq, params.Nu);
            penalty_scale = params.penalty_scale;
        }

        std::vector<std::shared_ptr<GlobalConstraints>> ptr_cons;  // Shared pointer to the constraints

        AL();                                           // Constructor

        double StageMerit(const int i, const double cost);

        void ALGradHess(const int k, MatrixXd& Lxx, MatrixXd& Luu, MatrixXd& Lux,
                VectorXd& Lx, VectorXd& Lu, const VectorXd& lxx, const VectorXd& luu,
                const VectorXd& lux, const VectorXd& lx, 
                const VectorXd& lu, const VectorXd& x, const VectorXd& u);

        // void ALGrad(VectorXd& Lx, VectorXd& Lu, const VectorXd& lx, 
        //         const VectorXd& lu,const VectorXd& x, const VectorXd& u);

        // void ALHess(MatrixXd& Lxx, MatrixXd& Luu, MatrixXd& Lux, 
        //         const VectorXd& lxx, const VectorXd& luu, const VectorXd& lux);
        
        void ActiveConstraint(const int k);

        void ConcatConstraint(const VectorXd& x, const VectorXd& u);

        void DualUpdate(const std::vector<VectorXd>& x_k, const std::vector<VectorXd>& u_k);
        
        void PenaltySchedule();
        // void ConcatConstraintJacob(const VectorXd& x, const VectorXd& u);


    private:
        double mu;
        MatrixXd I_mu;                                  // inq vs inq where inq is number of inequality constraints
        int inq;
        MatrixXd cx,cu;
        VectorXd c;
        std::vector<VectorXd> lambda;               // Dual variable for ALL agents (here we are assuming no equality constraints)
        double penalty_scale;
};