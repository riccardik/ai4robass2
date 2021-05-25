/************************************************************************
 * Copyright 2009, 2011, Strathclyde Planning Group,
 * Department of Computer and Information Sciences,
 * University of Strathclyde, Glasgow, UK
 * http://planning.cis.strath.ac.uk/
 *
 * Amanda Coles, Andrew Coles, Maria Fox, Derek Long - POPF
 * Maria Fox, Richard Howey and Derek Long - VAL
 * Stephen Cresswell - PDDL Parser
 *
 * This file is part of POPF.
 *
 * POPF is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * POPF is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with POPF.  If not, see <http://www.gnu.org/licenses/>.
 *
 ************************************************************************/

#include "FFEvent.h"
#include "RPGBuilder.h"

#ifdef STOCHASTICDURATIONS
#include "StochasticDurations.h"
#endif

#include <cassert>
#include <limits>
#include <sstream>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sstream>
#include <map>

using std::ostringstream;
using std::endl;

#include <sys/times.h>

namespace Planner {
    
    int FFEvent::tilLimit = 0;
    
    FFEvent::~FFEvent() {
#ifdef STOCHASTICDURATIONS
        delete stochasticTimestamp;
#endif
    }
    
    FFEvent::FFEvent(instantiatedOp* a, const double & dMin, const double & dMax)
    : action(a), time_spec(VAL::E_AT_START), minDuration(dMin), maxDuration(dMax), pairWithStep(-1),
    getEffects(true),
    lpTimestamp(-1.0),lpMinTimestamp(-1.0), lpMaxTimestamp(std::numeric_limits<double>::max()),
    divisionID(-1)
    {
        //cout << "FFEvent start\n";
#ifdef STOCHASTICDURATIONS
        stochasticTimestamp = 0;
#endif
        
    }
    
    FFEvent::FFEvent(instantiatedOp* a, const int & pw, const double & dMin, const double & dMax)
    : action(a), time_spec(VAL::E_AT_END), minDuration(dMin), maxDuration(dMax), pairWithStep(pw),
    getEffects(true), lpTimestamp(-1.0), /*lpEndTimestamp(-1.0), */lpMinTimestamp(-1.0), lpMaxTimestamp(std::numeric_limits<double>::max()), divisionID(-1)
    {
        //cout << "FFEvent end\n";
#ifdef STOCHASTICDURATIONS
        stochasticTimestamp = 0;
#endif
        
    }
    
    FFEvent::FFEvent(instantiatedOp* a, const int & s, const int & pw, const double & dMin, const double & dMax)
    : action(a), time_spec(VAL::E_OVER_ALL), minDuration(dMin), maxDuration(dMax), pairWithStep(pw),
    getEffects(true), lpTimestamp(-1.0), /*lpEndTimestamp(-1.0), */lpMinTimestamp(-1.0), lpMaxTimestamp(std::numeric_limits<double>::max()), divisionID(s)
    {
        //cout << "FFEvent end\n";
#ifdef STOCHASTICDURATIONS
        stochasticTimestamp = 0;
#endif
        
    }
    
    FFEvent::FFEvent(const int & t)
    : action(0), time_spec(VAL::E_AT), minDuration(-1.0), maxDuration(-1.0), pairWithStep(-1), getEffects(true),
    lpTimestamp(-1.0), /*lpEndTimestamp(-1.0), */lpMinTimestamp(-1.0), lpMaxTimestamp(std::numeric_limits<double>::max()), divisionID(t)
    {
        assert(divisionID <= tilLimit);
        //cout << "FFEvent start\n";
#ifdef STOCHASTICDURATIONS
        stochasticTimestamp = 0;
#endif
        
    }
    
    
    FFEvent::FFEvent(const FFEvent & f)
    : action(f.action), time_spec(f.time_spec), minDuration(f.minDuration), maxDuration(f.maxDuration),
    pairWithStep(f.pairWithStep), getEffects(f.getEffects) , lpTimestamp(f.lpTimestamp),
    lpMinTimestamp(f.lpMinTimestamp), lpMaxTimestamp(f.lpMaxTimestamp), divisionID(f.divisionID), needToFinish(f.needToFinish), fluents(f.fluents)
    {
        //cout << "FFEvent copy\n";
#ifdef STOCHASTICDURATIONS
        stochasticTimestamp = (f.stochasticTimestamp ? f.stochasticTimestamp->clone() : 0);
#endif
        
    }
    
    FFEvent::FFEvent()
    : action(0), time_spec(VAL::E_AT_START), minDuration(0.0), maxDuration(0.0), lpTimestamp(-1.0),
    lpMinTimestamp(-1.0), lpMaxTimestamp(std::numeric_limits<double>::max()), divisionID(-1)
    {
        //cout << "FFEvent default\n";
#ifdef STOCHASTICDURATIONS
        stochasticTimestamp = 0;
#endif
        
    }
    
    FFEvent & FFEvent::operator=(const FFEvent & f)
    {
        //cout << "FFEvent assignment op\n";
        action = f.action;
        time_spec = f.time_spec;
        minDuration = f.minDuration;
        maxDuration = f.maxDuration;
        pairWithStep = f.pairWithStep;
        getEffects = f.getEffects;
        lpTimestamp = f.lpTimestamp;
        lpMinTimestamp = f.lpMinTimestamp;
        lpMaxTimestamp = f.lpMaxTimestamp;
        divisionID = f.divisionID;
        needToFinish = f.needToFinish;
        fluents = f.fluents;
        
#ifdef STOCHASTICDURATIONS
        delete stochasticTimestamp;
        stochasticTimestamp = (f.stochasticTimestamp ? f.stochasticTimestamp->clone() : 0);
#endif
        
        return *this;
    }
    
    
    string threeDP(double d)
    {
        ostringstream toReturn;
        
        d *= 1000;
        
        int asInt = d;
        
        d -= asInt;
        if (d >= 0.5) {
            asInt += 1;
        }
        
        int fractionalPart = asInt % 1000;
        
        toReturn << asInt / 1000 << ".";
        
        if (fractionalPart < 100) {
            toReturn << "0";
        }
        if (fractionalPart < 10) {
            toReturn << "0";
        }
        
        toReturn << asInt % 1000;
        
        return toReturn.str();
    }
    
    void FFEvent::printPlan(const list<FFEvent> & toPrint)
    {
        tms refReturn;
        times(&refReturn);
        
        double secs = ((double)refReturn.tms_utime + (double)refReturn.tms_stime) / ((double) sysconf(_SC_CLK_TCK));
        
        int twodp = (int)(secs * 100.0);
        int wholesecs = twodp / 100;
        int centisecs = twodp % 100;
        
        cout << "; Time " << wholesecs << ".";
        if (centisecs < 10) cout << "0";
        cout << centisecs << "\n";
        list<FFEvent>::const_iterator planItr = toPrint.begin();
        const list<FFEvent>::const_iterator planEnd = toPrint.end();
        const int planSize = toPrint.size();
        vector<double> endTS(planSize);
#ifdef STOCHASTICDURATIONS
        vector<double> endSTS(planSize);
#endif
        vector<const FFEvent*> planVector(planSize);
        map<double, list<int> > sorted;
        for (int i = 0; planItr != planEnd; ++planItr, ++i) {
            if (planItr->time_spec == VAL::E_AT_START) {
                sorted[planItr->lpTimestamp].push_back(i);
                planVector[i] = &(*planItr);
            } else if (planItr->time_spec == VAL::E_AT_END) {
                endTS[i] = planItr->lpTimestamp;
#ifdef STOCHASTICDURATIONS
                endSTS[i] = planItr->stochasticTimestamp->getTimestampForRPGHeuristic();
#endif
            }
        }
        map<double, list<int> >::iterator sortedItr = sorted.begin();
        const map<double, list<int> >::iterator sortedEnd = sorted.end();
        
        for (; sortedItr != sortedEnd; ++sortedItr) {
            list<int>::iterator iItr = sortedItr->second.begin();
            const list<int>::iterator iEnd = sortedItr->second.end();
            
            for (; iItr != iEnd; ++iItr) {
                const FFEvent * const planItr = planVector[*iItr];
                if (planItr->lpTimestamp < 0.0000001) {
                    cout << "0.000";
                } else {
                    cout << threeDP(planItr->lpTimestamp);
                }
                cout << ": " << *(planItr->action) << " ";
                if (planItr->pairWithStep >= 0) {
                    const double dur = endTS[planItr->pairWithStep] - planItr->lpTimestamp;
                    cout << " [" << threeDP(dur) << "]";
#ifdef STOCHASTICDURATIONS
                    cout << ";\t\t {" << planItr->stochasticTimestamp->getTimestampForRPGHeuristic() << "} {" << endSTS[planItr->pairWithStep] << "}";
#endif
                } else if (RPGBuilder::getRPGDEs(planItr->action->getID()).empty()) {
                    cout << " [" << threeDP(RPGBuilder::getNonTemporalDurationToPrint()[planItr->action->getID()]) << "]";
#ifdef STOCHASTICDURATIONS
                    cout << ";\t\t {" << planItr->stochasticTimestamp->getTimestampForRPGHeuristic() << "} {" << EPSILON + planItr->stochasticTimestamp->getTimestampForRPGHeuristic() << "}";
#endif
                } else {
                    assert(false);
                }
                cout << endl;
            }
        }
    }
    void FFEvent::printVariables(const list<FFEvent> & toPrint, string nameOutput)
    {
        tms refReturn;
        times(&refReturn);
        
        double secs = ((double)refReturn.tms_utime + (double)refReturn.tms_stime) / ((double) sysconf(_SC_CLK_TCK));
        
        int twodp = (int)(secs * 100.0);
        int wholesecs = twodp / 100;
        int centisecs = twodp % 100;
        
        list<FFEvent>::const_iterator planItr = toPrint.begin();
        const list<FFEvent>::const_iterator planEnd = toPrint.end();
        const int planSize = toPrint.size();
        vector<double> endTS(planSize);
        vector<const FFEvent*> planVector(planSize);
        map<double, list<int> > sorted;
        for (int i = 0; planItr != planEnd; ++planItr, ++i) {
            sorted[planItr->lpTimestamp].push_back(i);
            planVector[i] = &(*planItr);
        }
        
        
        std::ofstream ofs (nameOutput.c_str(),  std::ofstream::out);
        
        map<double, list<int> >::iterator sortedItr = sorted.begin();
        const map<double, list<int> >::iterator sortedEnd = sorted.end();
        ofs <<",,";
        for (int v = 0; v<RPGBuilder::getPNECount() ; v++){
            ofs <<*(RPGBuilder::getPNE(v))<<",";
        }
        ofs << endl;
        for (; sortedItr != sortedEnd; ++sortedItr) {
            list<int>::iterator iItr = sortedItr->second.begin();
            const list<int>::iterator iEnd = sortedItr->second.end();
            for (; iItr != iEnd; ++iItr) {
                const FFEvent * const planItr = planVector[*iItr];
                //if (planItr->time_spec == VAL::E_AT_START || planItr->time_spec == VAL::E_AT_END)
                {
                    if (planItr->lpTimestamp < 0.0000001) {
                        ofs << "0.000";
                    } else {
                        ofs << threeDP(planItr->lpTimestamp);
                    }
                    ofs << ",";
                    if (planItr->time_spec == VAL::E_AT_START || planItr->time_spec == VAL::E_AT_END) {
                        ofs << *(planItr->action) << ",";
                    }else{
                        ofs << "TIL " << planItr->divisionID << ",";
                    }
                    for (int v = 0; v<RPGBuilder::getPNECount() ; v++){
                        double t = -1;
                        if (planItr->fluents.find(v)!=planItr->fluents.end())
                            t = planItr->fluents.find(v)->second;
                        ofs << t << ",";
                    }
                    ofs << endl;
                }
            }
        }
        ofs.close();  
    }
    string FFEvent::toString(){
        string toReturn;
        if(time_spec!=VAL::E_AT){
            toReturn =  action->getHead()->getName();
            if(time_spec==VAL::E_AT_START){
                toReturn += " at start";
            }else{
                toReturn += " at end";
            }
        }else{
            toReturn = "TIL ";
        }
        return toReturn;
    }
    
    void FFEvent::recordFluents(MinimalState state){
        for (int v = 0; v<RPGBuilder::getPNECount() ; v++){
            fluents[v]=state.secondMin[v];
        }
    }
    
};

