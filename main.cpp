﻿// An Example for Lagrangian Relaxation 
/************************************************************************************
 * Maximize 16X1 + 10X2 + 4X4
 * s.t.:
 * 8x1 + 2x2 + x3 + 4x4 <= 10
 * x1 + x2 <= 1
 * x3 + x4 <= 1
 * 𝑥𝑗 ∈ {0, 1}, j=1, 2, 3, 4
 ***********************************************************************************/

 ////USING CUTTING PLANE METHOD - RELAXING TWO VARIABLE////

#pragma warning(disable : 4996) //For Visual Studio 2012
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <fstream>
#include <iosfwd>
#include <string>
#include <deque>
#include <sstream>
#include <time.h>
#include <stdlib.h>
#include <vector> //for vectors
#include <math.h>

#include <ilcplex/ilocplex.h>
#include <ilconcert/ilosys.h>

using namespace std;

ILOSTLBEGIN

int main(int argc, char** argv)
{
	IloEnv env;
	try
	{
		////////DECISION VARIABLES AND PARAMETERS FOR SUBPROBLEM///////////////
		IloNumVarArray X(env, 4, 0, IloInfinity, ILOBOOL);
		IloNumArray U_val(env, 2);
		IloNum theta_val;

		////////DECISION VARIABLES AND PARAMETERS FOR MASTER PROBLEM///////////
		IloNumVarArray U(env, 2, 0, IloInfinity, ILOFLOAT);
		IloNumArray X_val(env, 4);
		//IloNumVar theta_var(env, -IloInfinity, IloInfinity, ILOFLOAT);
		IloNumVar theta_var(env, 0, IloInfinity, ILOFLOAT);

		//////////DEVELOP GENERIC MODEL //////////////////////////

		//////SET MASTER PROBLEM///////////////////////////
		IloModel model_master(env);
		IloExpr Objective_master(env);
		Objective_master = theta_var + U[0] + U[1];
		model_master.add(IloMinimize(env, Objective_master));
		IloCplex cplex_master(env);
		Objective_master.end();
		cplex_master.setOut(env.getNullStream()); // This is to supress the output of Branch & Bound Tree on screen
		cplex_master.setWarning(env.getNullStream()); //This is to supress warning messages on screen

		/////////SET SUBPROBLEM//////////////////////
		IloModel model_sub(env);
		IloObjective Objective_sub = IloMaximize(env);
		model_sub.add(Objective_sub);

		model_sub.add(8 * X[0] + 2 * X[1] + X[2] + 4 * X[3] <= 10);


		IloCplex cplex_sub(model_sub);
		IloNum eps = cplex_sub.getParam(IloCplex::EpInt);//Integer tolerance for MIP models; 
		//default value of EpInt remains 1e-5 http://www.iro.umontreal.ca/~gendron/IFT6551/CPLEX/HTML/relnotescplex/relnotescplex12.html
		cplex_sub.setOut(env.getNullStream()); // This is to supress the output of Branch & Bound Tree on screen
		cplex_sub.setWarning(env.getNullStream()); //This is to supress warning messages on screen

		/////////BEGIN ITERATIONS/////////////////////////////////
		IloNum GAP = IloInfinity;
		theta_val = 0;
		U_val[0] = 0;
		U_val[1] = 0;
		IloNum sub_obj_val = 0;
		IloNum Upper_bound = IloInfinity;
		IloNum Lower_bound = 0;
		GAP = Upper_bound - Lower_bound;
		cout << "U1 = " << U_val[0] << ", U2 = " << U_val[1] << endl;
		IloNum Iter = 0;
		IloNum	step_size = 6;
		IloNum step_size_gap = 1;

		//while( Iter < MaxCut )
		while (step_size_gap > 0.002)
		//while (GAP > 1)
		//while (Upper_bound - Lower_bound > eps)
		{
			Iter++;
			cout << "=========================================" << endl;
			cout << "============ITERATION " << Iter << "==============" << endl;
			//Define Object Function for the Dual of the Sub problem
			IloExpr sub_obj(env);
			sub_obj = (16 - U_val[0]) * X[0] + (10 - U_val[0]) * X[1] + (0 - U_val[1]) * X[2] + (4 - U_val[1]) * X[3];
			Objective_sub.setExpr(IloMaximize(env, sub_obj));
			//cplex_sub.setParam(cplex_sub.PreInd, 0);   //Disable presolve, otherwise, if dual is infeasible, 
																   //we don't know if prime is unbounded or infeasible
			cout << "SOLVING SUB PROBLEM" << endl;
			cplex_sub.solve();
			cout << "Sub Problem Solution Status: " << cplex_sub.getCplexStatus() << endl;
			if (cplex_sub.getCplexStatus() == CPX_STAT_OPTIMAL)
			{// Dual subproblem is bounded; Add Optimality Cut to the Master Problem
				cout << "U: " << U_val << endl;

				cplex_sub.getValues(X_val, X);  // taking values of X from SP and saves to X_val
				cout << "X_values = " << X_val << endl;
				//sub_obj_val = cplex_sub.getObjValue();
				//cout << "sub_obj_val = " << sub_obj_val << endl;
				//Upper_bound = IloMin(Upper_bound, (10 * U_val + sub_obj_val));
				//cout << "Upper_bound = " << Upper_bound << endl;

				//Add Cut to the Master Problem
				//cout << "Cut Added to Master Problem: " << "theta + " << (8 * X_val[0] + 2 * X_val[1] + X_val[2] + 4 * X_val[3]) << " U >= " << 16 * X_val[0] + 10 * X_val[1] + 4 * X_val[3] << endl;
				//model_master.add(theta_var + (8 * X_val[0] + 2 * X_val[1] + X_val[2] + 4 * X_val[3]) * U >= (16 * X_val[0] + 10 * X_val[1] + 4 * X_val[3]));

				Upper_bound = IloMin(Upper_bound, ((U_val[0] + U_val[1]) + (16 - U_val[0]) * X_val[0] + (10 - U_val[0]) * X_val[1] + (0 - U_val[1]) * X_val[2] + (4 - U_val[1]) * X_val[3]));
				cout << "Upper_bound = " << Upper_bound << endl;

				Lower_bound = 16 * X_val[0] + 10 * X_val[1] + 4 * X_val[3];
				cout << "Lower_bound = " << Lower_bound << endl;

				GAP = Upper_bound - Lower_bound;
				cout << "Gap = " << GAP << endl;



			}
			U_val[0] = IloMax(0, (U_val[0] - (step_size * (1 - X_val[0] - X_val[1]))));
			U_val[1] = IloMax(0, (U_val[0] - (step_size * (1 - X_val[2] - X_val[3]))));


			cout << "Step Size: " << step_size << endl;
			step_size_gap = step_size - (step_size / 2);
			step_size = step_size / 2;

			//if (Iter != 1) {
			//	step_size_gap = step_size - (1 / Iter);
			//	step_size = 1 / Iter;
			//}



			//cout << "SOLVING MASTER PROBLEM" << endl;
			//cout << "Master Problem Solution Status: " << cplex_master.getCplexStatus() << endl;
			//cplex_master.extract(model_master);
			//if (!cplex_master.solve())
			//{
			//	cout << "Failed" << endl;
			//	throw(-1);
			//}
			//U_val = cplex_master.getValue(U);
			//theta_val = cplex_master.getValue(theta_var);
			//cout << "theta_var = " << theta_val << endl;
			//cout << "U = " << U_val << endl;
			//Lower_bound = 10 * U_val + theta_val;
			//cout << "Lower_bound = " << Lower_bound << endl;
		}//while(Upper_bound - Lower_bound > eps)
		model_master.end();
		model_sub.end();
		cplex_master.end();
		cplex_sub.end();
	}//try
	catch (IloException& e)
	{
		env.out() << "ERROR: " << e << endl;
	}
	catch (...)
	{
		env.out() << "Unknown exception" << endl;
	}
	env.end();
	return 0;
}