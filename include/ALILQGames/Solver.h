#pragma once
//#include <eigen3/Eigen/Dense>
#include <Eigen/Dense>
#include <iostream>
#include <cassert>
#include <memory>

using Eigen::VectorXd;
using Eigen::MatrixXd;


struct Solver
{

    virtual ~Solver() = default;


    virtual void initial_rollout(const VectorXd& x0) = 0;

    virtual void forward_rollout(const VectorXd& x0) = 0;

    virtual void backward_pass() = 0;

    virtual void BackTrackingLineSearch(const VectorXd& x0) {}

    virtual void ArmuijoLineSearch(const VectorXd& x0) {}

    virtual void solve(const VectorXd& x0) = 0;

    virtual void recedingHorizon(const VectorXd& x0) {}

    virtual VectorXd getState(const int k) = 0;

    virtual VectorXd getControl(const int k) = 0;


    virtual double getStageCost(const int i, const int k) = 0;

    virtual double getTerminalCost(const int i) = 0;

};