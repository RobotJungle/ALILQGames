#pragma once

#include "model.h"
#include "SolverParams.h"

// State x: [x, y, theta, v]
// Input u: [v\dot, omega]

class DiffDriveModel4D : public Model {

    public:

        DiffDriveModel4D(SolverParams& params)
        {
            nx = 4;
            nu = 2;
            assert(nx == params.nx);
            assert(nu == params.nu);
            dt = params.dt;
            xdot = VectorXd::Zero(nx);
            discretizer = params.discetizer;

        }
    
        VectorXd dynamics(const VectorXd &x, const VectorXd &u) override {
            
            // VectorXd xdot(nx);
            xdot(0) = x(3)*std::cos(x(2));
            xdot(1) = x(3)*std::sin(x(2));
            xdot(2) = u(1);
            xdot(3) = u(0);
            return xdot;
        }

        void stateJacob(Eigen::Ref<MatrixXd> fx, const VectorXd& x, const VectorXd& u) override {
            assert(fx.rows() == nx);
            assert(fx.cols() == nx);

            fx <<  
                0.0, 0,  -x(3)*std::sin(x(2)), std::cos(x(2)), 
                0.0, 0,   x(3)*std::cos(x(2)), std::sin(x(2)),
                0.0, 0,                     0,              0,
                0.0, 0,                     0,              0;
            
            // case "RK4"
            // Discretize Jacobian
            fx = fx*dt + MatrixXd::Identity(nx, nx);

            // switch (discretizer)
            // {
            // case "Euler":
            //     fx = fx*dt + MatrixXd::Identity(nx, nx);
            //     break;
            
            // default:
            //     break;
            // }
        }

        void controlJacob(Eigen::Ref<MatrixXd> fu, const VectorXd& x, const VectorXd& u) override {
            assert(fu.rows() == nx);
            assert(fu.cols() == nu);

            fu <<  
                0.0,   0.0, 
                0.0,   0.0,
                0.0,   1.0,
                1.0,   0.0;
            
            // Discretize Jacobian
            fu = fu*dt;
        }
    private:
        VectorXd xdot;
        std::string discretizer;
};