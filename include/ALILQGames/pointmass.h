#pragma once

#include "model.h"
#include "SolverParams.h"


// State x: [x, y, ẋ, ẏ]
// Input u: [FX, Fy]

class PointMass : public Model {

    public:

        // ~PointMass() {}

        PointMass(SolverParams& params)
        {
            nx = 4;
            nu = 2;
            assert(nx == params.nx);
            assert(nu == params.nu);
            dt = params.dt;
        }
    
        VectorXd dynamics(const VectorXd& x, const VectorXd& u) override {
            VectorXd xdot(nx);
            xdot(0) = x(2);
            xdot(1) = x(3);
            xdot(2) = -(damp/mass)*x(2) + u(0)/mass;
            xdot(3) = -(damp/mass)*x(3) + u(1)/mass;
            return xdot;
        }


        void stateJacob(Eigen::Ref<MatrixXd> fx, const VectorXd& x, const VectorXd& u) override {
            assert(fx.rows() == nx);
            assert(fx.cols() == nx);

            fx <<  
                0.0, 0.0,          1.0,          0.0,
                0.0, 0.0,          0.0,          1.0,
                0.0, 0.0, (-damp/mass),          0.0,
                0.0, 0.0,          0.0, (-damp/mass);
            
            // Discretize Jacobian
            fx = fx*dt + MatrixXd::Identity(nx, nx);
        }

        void controlJacob(Eigen::Ref<MatrixXd> fu, const VectorXd& x, const VectorXd& u) override {
            assert(fu.rows() == nx);
            assert(fu.cols() == nu);

            fu <<  
                    0.0,      0.0, 
                    0.0,      0.0,
               (1/mass),      0.0, 
                    0.0, (1/mass);
            
            // Discretize Jacobian
            fu = fu*dt;
        }
    
    private:
        float mass = 1.0;
        float damp = 0.1;

};