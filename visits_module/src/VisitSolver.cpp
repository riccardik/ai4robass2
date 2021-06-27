/*
     <one line to give the program's name and a brief idea of what it does.>
     Copyright (C) 2015  <copyright holder> <email>
     
     This program is free software: you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     (at your option) any later version.
     
     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.
     
     You should have received a copy of the GNU General Public License
     along with this program.  If not, see <http://www.gnu.org/licenses/>.
     */

#include "VisitSolver.h"
#include "ExternalSolver.h"
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <math.h> 

#include "armadillo"
#include <initializer_list>
#include <random>




using namespace std;
using namespace arma;

//map <string, vector<double> > region_mapping;

extern "C" ExternalSolver *create_object()
{
  return new VisitSolver();
}

extern "C" void destroy_object(ExternalSolver *externalSolver)
{
  delete externalSolver;
}

VisitSolver::VisitSolver() {}

VisitSolver::~VisitSolver() {}

void VisitSolver::loadSolver(string *parameters, int n)
{
  starting_position = "r0";
  string Paramers = parameters[0];

  char const *x[] = {"actcost", "dummy"};
  char const *y[] = {"act-cost", "triggered"};
  parseParameters(Paramers);
  affected = list<string>(x, x + 1);
  dependencies = list<string>(y, y + 2);

  string waypoint_file = "/mnt/c/Users/rick/Desktop/visits/visits_domain/waypoint.txt";
  parseWaypoint(waypoint_file);

  string landmark_file = "/mnt/c/Users/rick/Desktop/visits/visits_domain/landmark.txt";
  parseLandmark(landmark_file);

  //startEKF();
}

map<string, double> VisitSolver::callExternalSolver(map<string, double> initialState, bool isHeuristic)
{

  map<string, double> toReturn;
  map<string, double>::iterator iSIt = initialState.begin();
  map<string, double>::iterator isEnd = initialState.end();
  double dummy;
  double act_cost;
  double asdf;
  double totalcost;
  double actcost;

  map<string, double> trigger;
  double res2 = 0;

  for (; iSIt != isEnd; ++iSIt)
  {

    string parameter = iSIt->first;
    string function = iSIt->first;
    double value = iSIt->second;

    function.erase(0, 1);
    function.erase(function.length() - 1, function.length());
    int n = function.find(" ");

    

    if (n != -1)
    {
      string arg = function;
      string tmp = function.substr(n + 1, 5);

      function.erase(n, function.length() - 1);
      arg.erase(0, n + 1);
      if (function == "triggered")
      {
        trigger[arg] = value > 0 ? 1 : 0;
        if (value > 0)
        {

          string from = tmp.substr(0, 2); // from and to are regions, need to extract wps (poses)
          string to = tmp.substr(3, 2);

          // localize(from, to);
          cout <<to<<","<<from <<endl;
          
          
          string wp_from=region_mapping[from][0];

          string wp_to=region_mapping[to][0];

          double computed_distance = localize(wp_from, wp_to);

          
          res2 = computed_distance;
        }
      }
    }
    else
    {
      if (function == "dummy")
      {
        dummy = value;
      }
      else if (function == "act-cost")
      {
        act_cost = value;
      }
      else if (function == "actcost")
      {
        actcost = value;
      }  
      else if (function == "asdf")
      {
        asdf= value;
      } 
      else if (function == "totalcost")
      {
        totalcost = value;
      } 
      //else if(function=="dummy1"){
      //duy = value;
      ////cout << parameter << " " << value << endl;
      //}
    }
  }

  //double results = calculateExtern(dummy, act_cost);
  if (ExternalSolver::verbose)
  {
    //cout << "(dummy) " << results << endl;
  }
   //cout << "(dummy) " << results << endl;

  //toReturn["(dummy)"] = res2;
  toReturn["(actcost)"] = res2;
  res2=2;
  //toReturn["(act-cost)"] = res2;
  //toReturn["(dummy)"] = res2;
  //toReturn["(asdf)"] = res2;

  return toReturn;
}

list<string> VisitSolver::getParameters()
{

  return affected;
}

list<string> VisitSolver::getDependencies()
{

  return dependencies;
}

void VisitSolver::parseParameters(string parameters)
{

  int curr, next;
  string line;
  ifstream parametersFile(parameters.c_str());
  if (parametersFile.is_open())
  {
    while (getline(parametersFile, line))
    {
      curr = line.find(" ");
      string region_name = line.substr(0, curr).c_str();
      curr = curr + 1;
      while (true)
      {
        next = line.find(" ", curr);
        region_mapping[region_name].push_back(line.substr(curr, next - curr).c_str());
        if (next == -1)
          break;
        curr = next + 1;
        
      }

      //to understand the format of the structure
      //cout<<"name"<<region_name<<": ";
      //cout<<region_mapping[region_name][0]<<endl;
    }
  }
}

double VisitSolver::calculateExtern(double external, double total_cost)
{
  //float random1 = static_cast <float> (rand())/static_cast <float>(RAND_MAX);
  double cost = 2; //random1;
  return cost;
}

void VisitSolver::parseWaypoint(string waypoint_file)
{

  int curr, next;
  string line;
  double pose1, pose2, pose3;
  ifstream parametersFile(waypoint_file);
  if (parametersFile.is_open())
  {
    while (getline(parametersFile, line))
    {
      curr = line.find("[");
      string waypoint_name = line.substr(0, curr).c_str();

      curr = curr + 1;
      next = line.find(",", curr);

      pose1 = (double)atof(line.substr(curr, next - curr).c_str());
      curr = next + 1;
      next = line.find(",", curr);

      pose2 = (double)atof(line.substr(curr, next - curr).c_str());
      curr = next + 1;
      next = line.find("]", curr);

      pose3 = (double)atof(line.substr(curr, next - curr).c_str());

      waypoint[waypoint_name] = vector<double>{pose1, pose2, pose3};

      //to understand the format of the structure
      //cout<<"waypoint: "<<waypoint_name<<": ";
      //cout<<waypoint[waypoint_name][0]<<","<<waypoint[waypoint_name][1]<<","<<waypoint[waypoint_name][2]<<endl;
    }
  }
}

void VisitSolver::parseLandmark(string landmark_file)
{

  int curr, next;
  string line;
  double pose1, pose2, pose3;
  ifstream parametersFile(landmark_file);

  landmarkNumber = 0;
  if (parametersFile.is_open())
  {
    while (getline(parametersFile, line))
    {
      curr = line.find("[");
      string landmark_name = line.substr(0, curr).c_str();

      curr = curr + 1;
      next = line.find(",", curr);

      pose1 = (double)atof(line.substr(curr, next - curr).c_str());
      curr = next + 1;
      next = line.find(",", curr);

      pose2 = (double)atof(line.substr(curr, next - curr).c_str());
      curr = next + 1;
      next = line.find("]", curr);

      pose3 = (double)atof(line.substr(curr, next - curr).c_str());

      landmark[landmark_name] = vector<double>{pose1, pose2, pose3};
      
      landmarkNumber++;
      landmarkNames.push_back(landmark_name);
      cout << landmark_name << endl;
    }

    /* for (int i = 0; i < landmarkNumber; i++)
    {
      cout <<"landmark "<<i<<": " <<landmarkNames[i] << endl;
    } */
  }
}

double VisitSolver::localize( string wp_from, string wp_to){
  vector<double> wp_f = {waypoint[wp_from][0], waypoint[wp_from][1], waypoint[wp_from][2]};

  //cout<<"the waypoint of the \"from\" region "<<from<<" is "<<region_mapping[from][0]<<endl;
  //cout<<"waypoint coords:["<<waypoint[wp_from][0]<<","<<waypoint[wp_from][1]<<","<<waypoint[wp_from][2]<<"]"<<endl;

  
  vector<double> wp_t = {waypoint[wp_to][0], waypoint[wp_to][1], waypoint[wp_to][2]};

  //cout<<"the waypoint of the \"to\" region "<<to<<" is "<<region_mapping[to][0]<<endl;
  //cout<<"waypoint coords:["<<waypoint[wp_to][0]<<","<<waypoint[wp_to][1]<<","<<waypoint[wp_to][2]<<"]"<<endl;

  vector<double> distance ={wp_t[0]-wp_f[0],wp_t[1]-wp_f[1],wp_t[2]-wp_f[2]};
  //set_difference(wp_f.begin(), wp_f.end(), wp_t.begin(), wp_t.end(),inserter(distance, distance.begin()));
  //cout<<"distance between start and end wp:["<<distance[0]<<","<<distance[1]<<","<<distance[2]<<"]"<<endl;


  //euclidean distance between the waypoints
  double euc_d = sqrt(distance[0] * distance[0] + distance[1] * distance[1]);

  // we consider an omnidirectional robot (teta do not influence the heading of the robot, x y and teta can evolve independently)
  // velocity is considered as 1 unit/s and steps are considered each 0.2 [s]
  

  double steps_per_second = 5;
  int steps_ = ceil(euc_d * steps_per_second);
  double stepx = sqrt(pow(wp_t[0] - wp_f[0], 2))/steps_;
  double stepy = sqrt(pow(wp_t[1] - wp_f[1], 2))/steps_;


  

  vector<double> step = {stepx, stepy};
  //cout << "step " << stepx <<"  "<<stepy<< endl;

  //trajectory between points
  //p1(wp_f[0],wp_f[1]), p2(wp_t[0],wp_t[1])

  //actual state
  vector<double> state = {wp_f[0], wp_f[1]};
  //extimated state
  vector<double> cov_0 = {0.02, 0};
  vector<double> cov_1 = {0, 0.02};
  
  vector<double> p_0 = {0.02, 0};
  vector<double> p_1 = {0, 0.02};

  std::default_random_engine generator;
  // sensor measurement error is assumed constant (+-2%)
  std::normal_distribution<double> distributionx(0,0.02*stepx);
  std::normal_distribution<double> distributiony(0,0.02*stepy);

  // initial state incertanty
  std::normal_distribution<double> distribution_initial(0,0.02);

  vector<double> state_ext = {wp_f[0] + distribution_initial(generator), wp_f[1] + distribution_initial(generator)};

  double kg = 0.5;

  for (int i = 0; i < steps_; i++)
  {
    // Real state of the robot
    state[0] = state[0] + step[0];
    state[1] = state[1] + step[1];

    //state_ext[0] = state_ext[0] +step[0]+  step[0]*cov_0[0] + step[0]*cov_1[0];
    //state_ext[1] = state_ext[1] +step[1]+  step[1]*cov_0[1] + step[1]*cov_1[1];

    // Estimated state of the robot

    std::normal_distribution<double> distributionx(0,p_0[0]+p_0[1]);
    std::normal_distribution<double> distributiony(0,p_1[0]+p_1[2]);

    state_ext[0] = (distributionx(generator)+step[0])+ state_ext[0];
    state_ext[1] = (distributiony(generator)+step[1])+ state_ext[1];

    p_0 = {p_0[0] + cov_0[0], p_0[1] + cov_0[1]};
    p_1 = {p_1[0] + cov_1[0], p_1[1] + cov_1[1]};

    //check if the robot passes over a landmark
    string on_landmark = LandmarkCheck(state[0], state[1]);
    if (on_landmark !="false")
    {
      //cout <<on_landmark<< landmark[on_landmark][0] << landmark[on_landmark][1] endl;

      cout << state[0] << state[1] << on_landmark << endl;
      //cout << "landmark found" << endl;
      //update the robot position as the landmark one+some noise
      state_ext[0] = landmark[on_landmark][0] + distribution_initial(generator);
      state_ext[1] = landmark[on_landmark][1] + distribution_initial(generator);
    }

    //state_ext[0]

    //cout << "step " <<i<<"x="<<state[0]<<"y="<<state[1]<<"x_ext="<<state_ext[0]<<"y_ext="<<state_ext[1]<<endl;
  }

    //cout << "the euclidean distance between " << wp_from << " and " << wp_to << " is: " << euc_d << endl;

  //return euc_d;

  // compute the euclidean distance between starting point and extimated reached one
  vector<double> simulated_distance = {state_ext[0] - wp_f[0], state_ext[1] - wp_f[1]};
  double euc_dnew = sqrt(simulated_distance[0] * simulated_distance[0] + simulated_distance[1] * simulated_distance[1]);

  /* distance = {state[0] - wp_f[0], state[1] - wp_f[1]};
  double euc_dnew = sqrt(distance[0] * distance[0] + distance[1] * distance[1]); */
  //cout << "original eucd" << euc_d << "kf eucd" << euc_dnew << endl;
  return euc_dnew;
}

string VisitSolver::LandmarkCheck(double x, double y)
{
  for (int i = 0; i < landmarkNumber; i++)
  {
    // compute euclidean distance between the estimated positions of the robot and the landmarks
    double x_l = landmark[landmarkNames[i]][0];
    double y_l = landmark[landmarkNames[i]][1];

    double euc_d2 = sqrt(pow(x-x_l, 2) + pow(y-y_l, 2));
    
    if (euc_d2<0.02)
    {
      //cout << "landmark detected, eucd="<<euc_d2 << endl;
      return landmarkNames[i];
    } 
  }
  return "false";
}