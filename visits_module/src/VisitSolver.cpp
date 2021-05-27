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
          //cout<<*landmark[0,0]<<endl;
          vector<double> aasd = waypoint[to];
          //cout<<aasd[0]<<aasd[1]<<aasd[2]<<endl;
          //cout<<aasd[0]<<endl;
          /* cout<<"size landmark"<<waypoint.size()<<endl;
          cout<<"size aasd"<<aasd.size()<<endl; */
          
           /* for (int i = 0; i < landmark.size(); i++) {
            cout <<"landmark"<< landmark[i] << endl;
          } */
          /* for (int i = 0; i < waypoint.size(); i++) {
            cout <<"landmark"<< waypoint[i] << endl;
          } 
          for (int i = 0; i < aasd.size(); i++) {
            cout << "aasd"<<aasd[i] << endl;
          }*/




          
          string wp_from=region_mapping[from][0];
          vector<double> wp_f = {waypoint[wp_from][0], waypoint[wp_from][1], waypoint[wp_from][2]};

          /* cout<<"the waypoint of the \"from\" region "<<from<<" is "<<region_mapping[from][0]<<endl;
          cout<<"waypoint coords:["<<waypoint[wp_from][0]<<","<<waypoint[wp_from][1]<<","<<waypoint[wp_from][2]<<"]"<<endl; */

          string wp_to=region_mapping[to][0];
          vector<double> wp_t = {waypoint[wp_to][0], waypoint[wp_to][1], waypoint[wp_to][2]};

          /* cout<<"the waypoint of the \"to\" region "<<to<<" is "<<region_mapping[to][0]<<endl;
          cout<<"waypoint coords:["<<waypoint[wp_to][0]<<","<<waypoint[wp_to][1]<<","<<waypoint[wp_to][2]<<"]"<<endl; */

          vector<double> distance ={wp_t[0]-wp_f[0],wp_t[1]-wp_f[1],wp_t[2]-wp_f[2]};
          //set_difference(wp_f.begin(), wp_f.end(), wp_t.begin(), wp_t.end(),inserter(distance, distance.begin()));
          cout<<"distance between start and end wp:["<<distance[0]<<","<<distance[1]<<","<<distance[2]<<"]"<<endl;



          act_cost = act_cost+2;
          

          double euc_d = sqrt(distance[0]*distance[0]+distance[1]*distance[1]);

          cout<<"the euclidean distance between "<<wp_from<<" and " <<wp_to<<" is: "<<euc_d<<endl; 

          //res2=4;
          res2 = euc_d;

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

  double results = calculateExtern(dummy, act_cost);
  if (ExternalSolver::verbose)
  {
    cout << "(dummy) " << results << endl;
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
      
    }
  }
}

//void VisitSolver::localize( string from, string to){
//}
