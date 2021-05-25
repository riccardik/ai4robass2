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

#include "temporalanalysis.h"

#ifdef POPF3ANALYSIS
#include "numericanalysis.h"
#endif

using std::endl;

namespace Planner
{

vector<vector<pair<double, double> > > TemporalAnalysis::actionTSBounds;
LiteralSet TemporalAnalysis::initialState;

void TemporalAnalysis::dummyDeadlineAnalysis()
{

    const int actCount = RPGBuilder::getFixedDEs().size();
    actionTSBounds = vector<vector<pair<double, double> > >(actCount, vector<pair<double, double> >(2, pair<double, double>(0.0, DBL_MAX)));

};

map<int, list<pair<double, double> > > TemporalAnalysis::windows;

void TemporalAnalysis::processTILDeadlines()
{

    static const bool debug = (Globals::globalVerbosity & 32);
    if (debug) cout << "Performing TIL deadline analysis\n";

    const int actCount = RPGBuilder::getFixedDEs().size();

    actionTSBounds = vector<vector<pair<double, double> > >(actCount, vector<pair<double, double> >(2, pair<double, double>(0.0, DBL_MAX)));

    static const list<pair<double, double> > emptyList;

    map<int, double> lastAppearance;

    map<int, bool> windowable;

    {

        vector<double> initialFluents;

        RPGBuilder::getInitialState(initialState, initialFluents);

        const vector<RPGBuilder::FakeTILAction*> & allTILs = RPGBuilder::getTILVec();

        if (debug) {
            cout << "Number of TIL happenings: " << allTILs.size() << endl;            
        }
        
        map<int, double> literalAppears;
        map<int, double> literalDisappearsForGood;

        const int tilCount = allTILs.size();
        
        for (int tilID = 0; tilID < tilCount; ++tilID) {
            RPGBuilder::FakeTILAction * const tilItr = allTILs[tilID];
            
            if (debug) cout << "\tTIL " << tilID << " at " << tilItr->duration << "\n";

            {

                list<Literal*> & effs = tilItr->delEffects;

                list<Literal*>::iterator effItr = effs.begin();
                const list<Literal*>::iterator effEnd = effs.end();

                for (; effItr != effEnd; ++effItr) {
                    const int litID = (*effItr)->getStateID();
                    const pair<map<int, bool>::iterator, bool> wItr = windowable.insert(make_pair(litID, true));
                    if (wItr.second) {
                        const list<pair<int, VAL::time_spec> > & etaList = RPGBuilder::getEffectsToActions(litID);
                        list<pair<int, VAL::time_spec> >::const_iterator etItr = etaList.begin();
                        const list<pair<int, VAL::time_spec> >::const_iterator etEnd = etaList.end();

                        for (; etItr != etEnd; ++etItr) {
                            if (etItr->second != VAL::E_AT) {
                                if (debug) {
                                    if (wItr.first->second) {
                                        cout << *(*effItr) << " does not form windows, is added by:\n";
                                    }
                                }
                                wItr.first->second = false;
                                if (!debug) break;
                                if (debug) {
                                    if (etItr->second == VAL::E_AT_START) {
                                        cout << "\t" << *(RPGBuilder::getInstantiatedOp(etItr->first)) << " start\n";
                                    } else if (etItr->second == VAL::E_AT_END) {
                                        cout << "\t" << *(RPGBuilder::getInstantiatedOp(etItr->first)) << " end\n";
                                    }
                                }
                            }
                        }

                        if (debug && wItr.first->second) {
                            cout << *(*effItr) << " forms windows - only added by TILs\n";
                        }

                    }
                    if (wItr.first->second) {
                        literalDisappearsForGood.insert(pair<int, double>(litID, tilItr->duration));
                        list<pair<double, double> > & dest = windows[litID];

                        if (debug) {
                            cout << "Looking at window behaviour for " << *(*effItr);
                        }

                        const pair<map<int, double>::iterator, bool> laItr = lastAppearance.insert(make_pair(litID, DBL_MAX));
                        double & startOfWindow = laItr.first->second;

                        if (laItr.second) {
                            if (dest.empty()) {
                                if (initialState.find(*effItr) != initialState.end()) {
                                    startOfWindow = 0.0;
                                    if (debug) cout << " - true in the initial state";
                                } else {
                                    if (debug) cout << " - not true in the initial state";
                                }
                            } else {
                                if (debug) cout << " - not reappeared since last delete";
                            }
                        } else {
                            if (debug) cout << " - last appeared at " << startOfWindow;
                        }



                        if (startOfWindow != DBL_MAX) {
                            if (debug) {
                                cout << " - making a window [" << startOfWindow << "," << tilItr->duration << "]";
                            }
                            dest.push_back(make_pair(startOfWindow, tilItr->duration));
                        }

                        lastAppearance.erase(laItr.first);

                        if (debug) cout << " - done\n";
                    } else {

                    }
                }

            }

            {

                list<Literal*> & effs = tilItr->addEffects;

                list<Literal*>::iterator effItr = effs.begin();
                const list<Literal*>::iterator effEnd = effs.end();

                for (; effItr != effEnd; ++effItr) {

                    const int litID = (*effItr)->getStateID();
                    const pair<map<int, bool>::iterator, bool> wItr = windowable.insert(make_pair(litID, true));
                    if (wItr.second) {
                        const list<pair<int, VAL::time_spec> > & etaList = RPGBuilder::getEffectsToActions(litID);
                        list<pair<int, VAL::time_spec> >::const_iterator etItr = etaList.begin();
                        const list<pair<int, VAL::time_spec> >::const_iterator etEnd = etaList.end();

                        for (; etItr != etEnd; ++etItr) {
                            if (etItr->second != VAL::E_AT) {
                                if (debug) {
                                    if (wItr.first->second) {
                                        cout << *(*effItr) << " does not form windows, is added by:\n";
                                    }
                                }
                                wItr.first->second = false;
                                if (!debug) break;
                                if (debug) {
                                    if (etItr->second == VAL::E_AT_START) {
                                        cout << "\t" << *(RPGBuilder::getInstantiatedOp(etItr->first)) << " start\n";
                                    } else if (etItr->second == VAL::E_AT_END) {
                                        cout << "\t" << *(RPGBuilder::getInstantiatedOp(etItr->first)) << " end\n";
                                    }
                                }
                            }
                        }

                        if (debug && wItr.first->second) {
                            cout << *(*effItr) << " forms windows - only added by TILs\n";
                        }

                    }
                    if (wItr.first->second) {
                        literalDisappearsForGood.erase(litID);

                        const pair<map<int, double>::iterator, bool> laItr = lastAppearance.insert(make_pair(litID, 0.0));

                        if (laItr.second) {
                            double visibleFrom = tilItr->duration;
                            if (windows.find(litID) == windows.end()) { // appeared, but no time-windows defined yet
                                if (initialState.find(*effItr) != initialState.end()) visibleFrom = 0.0;
                            }
                            laItr.first->second = visibleFrom;
                        }

                        if (initialState.find(*effItr) == initialState.end() && RPGBuilder::getEffectsToActions((*effItr)->getStateID()).empty()) {
                            literalAppears.insert(pair<int, double>((*effItr)->getStateID(), tilItr->duration));
                        }
                    } else {
                        if (debug) {
                            cout << *(*effItr) << " does not form windows, can be added by:\n";
                            const list<pair<int, VAL::time_spec> > & etaList = RPGBuilder::getEffectsToActions(litID);
                            list<pair<int, VAL::time_spec> >::const_iterator etItr = etaList.begin();
                            const list<pair<int, VAL::time_spec> >::const_iterator etEnd = etaList.end();

                            for (; etItr != etEnd; ++etItr) {
                                if (etItr->second == VAL::E_AT_START) {
                                    cout << "\t" << *(RPGBuilder::getInstantiatedOp(etItr->first)) << " start\n";
                                } else if (etItr->second == VAL::E_AT_END) {
                                    cout << "\t" << *(RPGBuilder::getInstantiatedOp(etItr->first)) << " end\n";
                                } else if (etItr->second == VAL::E_AT) {
                                    cout << "\tTIL " << etItr->first << "\n";
                                }
                            }
                        }
                    }
                }
            }

        }

        {
            map<int, double>::iterator restrictItr = literalAppears.begin();
            const map<int, double>::iterator restrictEnd = literalAppears.end();

            for (; restrictItr != restrictEnd; ++restrictItr) {

                const double epsilonOff = restrictItr->second + EPSILON;

                list<pair<int, VAL::time_spec> > & affected = RPGBuilder::getPresToActions()[restrictItr->first];
                list<pair<int, VAL::time_spec> >::iterator affItr = affected.begin();
                const list<pair<int, VAL::time_spec> >::iterator affEnd = affected.end();

                for (; affItr != affEnd; ++affItr) {
                    if (affItr->second == VAL::E_AT_START) {
                        double & currMin = actionTSBounds[affItr->first][0].first;
                        if (epsilonOff > currMin) currMin = epsilonOff;

                        const double endOff = epsilonOff + RPGBuilder::getOpMinDuration(affItr->first, 0);
                        double & endMin = actionTSBounds[affItr->first][1].first;
                        if (endOff > endMin) endMin = endOff;
                    } else {
                        double & currMin = actionTSBounds[affItr->first][1].first;
                        if (epsilonOff > currMin) currMin = epsilonOff;

                        const double startOff = epsilonOff - RPGBuilder::getOpMaxDuration(affItr->first, 0);
                        double & startMin = actionTSBounds[affItr->first][0].first;
                        if (startOff > startMin) startMin = startOff;
                    }
                }
            }
        }

        {
            map<int, double>::iterator restrictItr = literalDisappearsForGood.begin();
            const map<int, double>::iterator restrictEnd = literalDisappearsForGood.end();

            for (; restrictItr != restrictEnd; ++restrictItr) {

                if (debug) cout << *(RPGBuilder::getLiteral(restrictItr->first)) << " disappears at " << restrictItr->second << "\n";

                const double epsilonOff = restrictItr->second - EPSILON;

                list<pair<int, VAL::time_spec> > & affected = RPGBuilder::getRawPresToActions()[restrictItr->first];

                if (debug) cout << "Bounding " << affected.size() << " start/end points\n";
                list<pair<int, VAL::time_spec> >::iterator affItr = affected.begin();
                const list<pair<int, VAL::time_spec> >::iterator affEnd = affected.end();

                for (; affItr != affEnd; ++affItr) {
                    if (affItr->second == VAL::E_AT_START) {
                        double & currMax = actionTSBounds[affItr->first][0].second;
                        if (debug) {
                            cout << "Latest of " << *(RPGBuilder::getInstantiatedOp(affItr->first)) << ", start, was = " << currMax;
                        }

                        if (epsilonOff < currMax) currMax = epsilonOff;

                        if (debug) {
                            cout << ", now = " << currMax << endl;
                        }

                        
                        const double endOff = epsilonOff + RPGBuilder::getOpMaxDuration(affItr->first, 0);
                        double & endMax = actionTSBounds[affItr->first][1].second;
                        
                        if (debug) {
                            cout << "Latest of " << *(RPGBuilder::getInstantiatedOp(affItr->first)) << ", end, was = " << endMax;
                        }
                        
                        if (endOff < endMax) endMax = endOff;
                        if (debug) {
                            cout << ", now = " << endMax << endl;
                        }
                        
                    } else { // invariant or at end
                        double & currMax = actionTSBounds[affItr->first][1].second;
                        if (epsilonOff < currMax) currMax = epsilonOff;

                        const double startOff = epsilonOff - RPGBuilder::getOpMinDuration(affItr->first, 0);
                        double & startMax = actionTSBounds[affItr->first][0].second;
                        if (startOff < startMax) startMax = startOff;
                    }
                }
            }
        }

        {
            map<int, double>::const_iterator laItr = lastAppearance.begin();
            const map<int, double>::const_iterator laEnd = lastAppearance.end();

            for (; laItr != laEnd; ++laItr) {
                list<pair<double, double> > & dest = windows[laItr->first];
                if (!dest.empty()) {
                    dest.push_back(make_pair(laItr->second, DBL_MAX));
                }
            }
        }
        {

            map<int, list<pair<double, double> > >::const_iterator winItr = windows.begin();
            const map<int, list<pair<double, double> > >::const_iterator winEnd = windows.end();

            for (; winItr != winEnd; ++winItr) {
                const list<pair<double, double> > & currList = winItr->second;
                if (currList.empty()) continue;

                if (debug) {
                    cout << "Windows on " << *(RPGBuilder::getLiteral(winItr->first)) << ":";
                    list<pair<double, double> >::const_iterator cwItr = currList.begin();
                    const list<pair<double, double> >::const_iterator cwEnd = currList.end();

                    for (; cwItr != cwEnd; ++cwItr) {
                        cout << " [" << cwItr->first << ",";
                        if (cwItr->second == DBL_MAX) {
                            cout << "end]";
                        } else {
                            cout << cwItr->second << "]";
                        }
                    }
                    cout << "\n";
                }
            }
        }
    }
    /*
    for (int i = 0; i < actCount; ++i) {
        if (!RPGBuilder::rogueActions[i]) {
            double & startMax = actionTSBounds[i][0].second;
            // if (startMax < DBL_MAX) cout << "Start of " << *(RPGBuilder::getInstantiatedOp(i)) << " is applicable no later than " << startMax << "\n";
        }
    }*/

};

void TemporalAnalysis::findGoalDeadlines(list<Literal*> & goals, list<double> & dest)
{
    
    list<Literal*>::iterator gItr = goals.begin();
    const list<Literal*>::iterator gEnd = goals.end();

    list<double>::iterator dItr = dest.begin();
    
    for (; gItr != gEnd; ++gItr, ++dItr) {

        double goalDeadline = 0.0;

        if (initialState.find(*gItr) == initialState.end()) {
            list<pair<int, VAL::time_spec> > & eta = RPGBuilder::getEffectsToActions((*gItr)->getStateID());

            list<pair<int, VAL::time_spec> >::iterator etaItr = eta.begin();
            const list<pair<int, VAL::time_spec> >::iterator etaEnd = eta.end();

            for (; etaItr != etaEnd; ++etaItr) {

                double thisDeadline = DBL_MAX;
                
                if (etaItr->second == VAL::E_AT) {
                    thisDeadline = RPGBuilder::getTILVec()[etaItr->first]->duration;
                } else if (etaItr->second == VAL::E_AT_START) {
                    thisDeadline = actionTSBounds[etaItr->first][0].second;
                } else {
                    thisDeadline = actionTSBounds[etaItr->first][1].second;
                }
                /*
                if (etaItr->second == VAL::E_AT) {
                    
                } else {
                    cout << *(RPGBuilder::getInstantiatedOp(etaItr->first));
                    
                    if (etaItr->second == VAL::E_AT_START) {
                        cout << " cannot start after " << thisDeadline << endl;
                    } else {
                        cout << " cannot end after " << thisDeadline << endl;
                    }
                
                }*/
                
                if (thisDeadline > goalDeadline) {
                    goalDeadline = thisDeadline;
                    if (goalDeadline == DBL_MAX) break;
                    goalDeadline += EPSILON;
                }
            }
        } else {
            goalDeadline = DBL_MAX;
        }
        /*if (goalDeadline != DBL_MAX) {
          cout << "Deadline on goal " << *(*gItr) << ": " << goalDeadline << "\n";
        } else {
          cout << "No deadline on goal " << *(*gItr) << "\n";
        }*/
        if (goalDeadline < *dItr) {
            *dItr = goalDeadline;
        }
    }
};

bool TemporalAnalysis::actionIsNeverApplicable(const int & a)
{
    if ((actionTSBounds[a][0].first > actionTSBounds[a][0].second) || (actionTSBounds[a][1].first > actionTSBounds[a][1].second)) {
        //cout << "Case A" << endl;
        return true;
    }
    if ((actionTSBounds[a][0].first + RPGBuilder::getOpMinDuration(a, -1)) > actionTSBounds[a][1].second) {
        //cout << "Case B" << endl;
        return true;
    }
    if ((actionTSBounds[a][1].first - RPGBuilder::getOpMaxDuration(a, -1)) > actionTSBounds[a][0].second) {
        //cout << "Case C" << endl;
        return true;
    }

    return false;
}

void TemporalAnalysis::findActionTimestampLowerBounds()
{

    RPGHeuristic* const currRPG = RPGBuilder::getHeuristic();

    LiteralSet initialState;
    vector<double> initialFluents;

    RPGBuilder::getInitialState(initialState, initialFluents);


    MinimalState refState;
    refState.insertFacts(initialState.begin(), initialState.end(), StepAndBeforeOrAfter());

    refState.secondMin = initialFluents;
    refState.secondMax = initialFluents;


//  refState.derivePredicates();

    currRPG->doFullExpansion(refState);

    
    
    const int actCount = RPGBuilder::actionsToStartPreconditions.size();

    for (int a = 0; a < actCount; ++a) {

        if (!RPGBuilder::rogueActions[a]) {

            bool isRogue = false;
            
            
            double earliestStart = RPGHeuristic::getEarliestForStarts()[a].toDouble();
            double earliestEnd = RPGHeuristic::getEarliestForEnds()[a].toDouble();

            if (earliestStart != DBL_MAX && earliestEnd != DBL_MAX) {
                //cout << "Initial RPG bounds " << *(getInstantiatedOp(a)) << " no earlier than " << earliestStart << " ; " << earliestEnd << "\n";
                const double maxActDur = RPGBuilder::getOpMaxDuration(a, -1);
                if (earliestStart < (earliestEnd - maxActDur) - 0.0005) {
                    earliestStart = earliestEnd - maxActDur;
//                  cout << "Maximum act duration = " << maxActDur << ", so pulling start to " << earliestStart << "\n";
                }

                TemporalAnalysis::suggestNewStartLowerBound(a, earliestStart);
                TemporalAnalysis::suggestNewEndLowerBound(a, earliestEnd);

                isRogue = TemporalAnalysis::actionIsNeverApplicable(a);

                if (isRogue) {
                    cout << "Pruning " << *(RPGBuilder::getInstantiatedOp(a)) << " - temporal contradiction\n";
                    #ifdef ENABLE_DEBUGGING_HOOKS
                        Globals::eliminatedAction(a, "The earliest point the start could be applied was too close to the latest point at which the end could be applied");
                    #endif
                }
            } else {
                cout << "Pruning " << *(RPGBuilder::getInstantiatedOp(a)) << " - never appeared in initial RPG\n";
                #ifdef ENABLE_DEBUGGING_HOOKS
                    Globals::eliminatedAction(a, "The action never appeared in the initial RPG");
                #endif
                isRogue = true;
            }

            if (isRogue) {

                RPGBuilder::pruneIrrelevant(a);

            }

        }

    }

}



/** Determine if the list of end preconditions is a subset of the action's invariants.
 *
 *  This is a helper function for compression-safe action detection, and is used
 *  to check whether an action's end precondition list is a subset of its invariants,
 *  one of the criteria used.
 *
 *  @param endPre  The end preconditions of an action
 *  @param inv     The invariants of an action
 *  @retval <code>true</code>  The end preconditions are a subset of the invariants
 *  @retval <code>false</code>  The end preconditions are not a subset of the invariants
 */
bool firstIsSubsumedBySecond(const list<Literal*> & endPre, const list<Literal*> & inv)
{
    list<Literal*>::const_iterator pItr = endPre.begin();
    const list<Literal*>::const_iterator pEnd = endPre.end();
    
    for (; pItr != pEnd; ++pItr) {
        list<Literal*>::const_iterator iItr = inv.begin();
        const list<Literal*>::const_iterator iEnd = inv.end();
        
        for (; iItr != iEnd; ++iItr) {
            if (*iItr == *pItr) break;
        }
        if (iItr == iEnd) return false;
    }
        
    return true;
}


/** @brief Make sure no actions depend on the facts in the given effects list.
 *
 *  This is a helper function for compression-safe action detection, and is used
 *  to check whether an action's end effects are only ever beneficial.  In the
 *  case of end delete effects, they must not be used as a positive precondition
 *  of any action, or as a goal.  In the case of end add effects, they must not
 *  be used as a negative precondition of an action, or as a negative goal.
 *
 *  @param effs     The end effects of an action
 *  @param affects  The lookup table from literal IDs to which snap-actions require
 *                  the fact as a precondition.  (If <code>effs</code> is the end
 *                  delete effects of an action, this should be a reference to 
 *                  <code>RPGBuilder::preconditionsToActions</code>.)
 *  @param goals    A set of goals, none of which can be made inviolate by <code>effs</code>.
 *  @param canIgnore  Any elements of <code>effects</code> that are in this set can be ignored.
 *  
 *  @retval <code>true</code>   There is no overlap between the effects list and the preconditions of actions or goals
 *  @retval <code>false</code>  The effects list can violate some preconditions and/or goals
 */
bool noOverlap(const list<Literal*> & effs, const vector<list<pair<int, VAL::time_spec> > > & affects,
               const LiteralSet & goals, const LiteralSet & canIgnore)
{
    list<Literal*>::const_iterator effItr = effs.begin();
    const list<Literal*>::const_iterator effEnd = effs.end();
    
    for (; effItr != effEnd; ++effItr) {
        if (canIgnore.find(*effItr) != canIgnore.end()) continue;
        
        if (!affects[(*effItr)->getStateID()].empty() || goals.find(*effItr) != goals.end()) {
            if (Globals::globalVerbosity & 16384) {
                cout << "Fact " << *(*effItr) << ": ";
            }
            return false;
        }
    }
    
    return true;
}

void TemporalAnalysis::recogniseHoldThroughoutDeleteAtEndIdiom(LiteralSet & factsIdentified)
{

    const vector<list<pair<int, VAL::time_spec> > > & preconditionsToActions = RPGBuilder::getRawPresToActions();
    const vector<list<pair<int, VAL::time_spec> > > & negativeEffectsToActions = RPGBuilder::negativeEffectsToActions;
    
    const int litCount = preconditionsToActions.size();

    // First, we exclude facts that could be added by a TIL during the action and then deleted again by its end.
    // In this case, moving the end delete to the start and scrubbing the invariant is not equivalent.
    
    set<int> exclude;
        
    {
        const vector<RPGBuilder::FakeTILAction*> & TILs = RPGBuilder::getAllTimedInitialLiterals();
        
        const int tilCount = TILs.size();
        
        for (int t = 0; t < tilCount; ++t) {
            list<Literal*>::const_iterator effItr = TILs[t]->addEffects.begin();
            const list<Literal*>::const_iterator effEnd = TILs[t]->addEffects.end();
            
            for (; effItr != effEnd; ++effItr) {
                exclude.insert((*effItr)->getStateID());
            }
        }
    }
    
    const set<int>::const_iterator exEnd = exclude.end();
    set<int>::const_iterator exItr = exclude.begin();
    
    for (int lit = 0; lit < litCount; ++lit) {
        
        if (exItr != exEnd) {
            if (*exItr == lit) { // skip any facts manipulated by TILs
                ++exItr;
                if (Globals::globalVerbosity & 16384) {
                    cout << *RPGBuilder::getLiteral(lit) << ", if end-deleted, will break the action being compression safe, due to potential TIL interactions\n";
                }
                
                continue;
            }
        }
        
        vector<set<int> > startEndDelete(3);
        
        list<pair<int, VAL::time_spec> >::const_iterator eItr = negativeEffectsToActions[lit].begin();
        const list<pair<int, VAL::time_spec> >::const_iterator eEnd = negativeEffectsToActions[lit].end();
        
        for (; eItr != eEnd; ++eItr) {
            switch (eItr->second) {
                case VAL::E_AT_START:
                    startEndDelete[0].insert(eItr->first);
                    break;
                case VAL::E_AT_END:
                    startEndDelete[1].insert(eItr->first);
                    break;
                case VAL::E_AT: {
                    startEndDelete[2].insert(eItr->first);
                    break;
                }
                default:
                {
                    std::cerr << "Fatal internal error: effect should be at start or at end.\n";
                    exit(1);
                }
            }
        }
        
        if (!startEndDelete[2].empty()) {
            continue;
        }
        
        if (startEndDelete[1].empty()) {
            //never end deleted, so no concerns
            continue;
        }
        
        vector<set<int> > actionsWithFactAsAStartInvEndCondition(3);
        
        list<pair<int, VAL::time_spec> >::const_iterator pItr = preconditionsToActions[lit].begin();
        const list<pair<int, VAL::time_spec> >::const_iterator pEnd = preconditionsToActions[lit].end();
        
        for (; pItr != pEnd; ++pItr) {
            switch (pItr->second) {
                case VAL::E_AT_START:
                    actionsWithFactAsAStartInvEndCondition[0].insert(pItr->first);
                    break;
                case VAL::E_OVER_ALL:
                    actionsWithFactAsAStartInvEndCondition[1].insert(pItr->first);
                    break;
                case VAL::E_AT_END:
                    actionsWithFactAsAStartInvEndCondition[2].insert(pItr->first);
                    break;
                default:
                {
                    std::cerr << "Fatal internal error: precondition recorded for a time specifier other than at start, over all or at end.\n";
                    exit(1);
                }
            }
        }


        set<int> onlyAsEnd;
        
        std::set_difference(actionsWithFactAsAStartInvEndCondition[2].begin(), actionsWithFactAsAStartInvEndCondition[2].end(),
                            actionsWithFactAsAStartInvEndCondition[1].begin(), actionsWithFactAsAStartInvEndCondition[1].end(),
                            insert_iterator<set<int> >(onlyAsEnd, onlyAsEnd.begin()));
                            
                            
        if (!onlyAsEnd.empty()) {
            // would not be compression safe, as there's an end-only precondition
            if (Globals::globalVerbosity & 16384) {
                cout << *RPGBuilder::getLiteral(lit) << ", if end-deleted, will break the action being compression safe, as it can be required only at the end of some action\n";
            }                                        
            continue;
        }
        // Actions which only have the fact as an 'at start' precondition - these must also delete at start
        
        set<int> onlyAsAStart;
        
        std::set_difference(actionsWithFactAsAStartInvEndCondition[0].begin(), actionsWithFactAsAStartInvEndCondition[0].end(),
                            actionsWithFactAsAStartInvEndCondition[1].begin(), actionsWithFactAsAStartInvEndCondition[1].end(),
                            insert_iterator<set<int> >(onlyAsAStart, onlyAsAStart.begin()));
                                
        
        
        set<int> onlyAsStartButNotThenDeleted;
        
        std::set_difference(onlyAsAStart.begin(), onlyAsAStart.end(),
                            startEndDelete[0].begin(), startEndDelete[0].end(),
                            insert_iterator<set<int> >(onlyAsStartButNotThenDeleted, onlyAsStartButNotThenDeleted.begin()));
        
        if (!onlyAsStartButNotThenDeleted.empty()) {
            // breaks the idiom: cannot move the end delete to the start of an action if something else could
            // use it only momentarily during its execution.
            if (Globals::globalVerbosity & 16384) {
                cout << *RPGBuilder::getLiteral(lit) << ", if end-deleted, will break the action being compression safe, as it can needed at the start of an action, but isn't then deleted\n";
            }                                        
                                    
            continue;
        }
        
        set<int> overAllButNotDeletedAtEnd;
        
        std::set_difference(actionsWithFactAsAStartInvEndCondition[1].begin(), actionsWithFactAsAStartInvEndCondition[1].end(),
                            startEndDelete[1].begin(), startEndDelete[1].end(),
                            insert_iterator<set<int> >(overAllButNotDeletedAtEnd, overAllButNotDeletedAtEnd.begin()));
        
        if (!overAllButNotDeletedAtEnd.empty()) {
            // breaks the idiom: could potentially use and not delete the fact after the start but before the end of an action which 
            // will delete the fact at its end
            if (Globals::globalVerbosity & 16384) {
                cout << *RPGBuilder::getLiteral(lit) << ", if end-deleted, will break the action being compression safe, as some action " << *RPGBuilder::getInstantiatedOp(*(overAllButNotDeletedAtEnd.begin())) << " needs it as an invariant, but doesn't then delete it\n";
            }                                        
            
            continue;
        }
        
        // otherwise, we've made it - all actions with a precondition on the fact either:
        //  i) delete it straight away
        // ii) need it throughout, and then delete it at the end
        
        factsIdentified.insert(RPGBuilder::getLiteral(lit));
        
        if (Globals::globalVerbosity & 16384) {
            cout << *RPGBuilder::getLiteral(lit) << " is end-deleted, but recognised as still being compression safe\n";
        }
    }
    
}

#ifdef POPF3ANALYSIS
bool TemporalAnalysis::endNumericEffectsAreCompressionSafe(const list<int> & effects,
                                                           vector<bool> & allInteractionWithVarIsCompressionSafe)
{
    
    
    list<int>::const_iterator effItr = effects.begin();
    const list<int>::const_iterator effEnd = effects.end();
    
    for (; effItr != effEnd; ++effItr) {
        
        const RPGBuilder::RPGNumericEffect & currEff = RPGBuilder::getNumericEff()[*effItr];
        
        // first, all current interaction with the variable has to be compression safe
        if (!allInteractionWithVarIsCompressionSafe[currEff.fluentIndex]) {
            if (Globals::globalVerbosity & 16384) {
                cout << "Interacts with non-compression safe variable " << *(RPGBuilder::getPNE(currEff.fluentIndex)) << endl;
            }
            return false;
        }
        
        // third, the effect has to be only ever a good idea, with respect to the dominance constraints on the variable
        switch (NumericAnalysis::getDominanceConstraints()[currEff.fluentIndex]) {
            case NumericAnalysis::E_NODOMINANCE: {
                if (Globals::globalVerbosity & 16384) {
                    cout << "Interacts with variable with no dominance constraints, " << *(RPGBuilder::getPNE(currEff.fluentIndex)) << endl;
                }                
                return false;
            }
            case NumericAnalysis::E_METRICTRACKING: {
                if (Globals::globalVerbosity & 16384) {
                    cout << "Interacts with metric-tracking variable, " << *(RPGBuilder::getPNE(currEff.fluentIndex)) << endl;
                }
                break;
            }
            case NumericAnalysis::E_IRRELEVANT: {
                if (Globals::globalVerbosity & 16384) {
                    cout << "Interacts with irrelevant variable, " << *(RPGBuilder::getPNE(currEff.fluentIndex)) << endl;
                }
                break;
            }
            case NumericAnalysis::E_SMALLERISBETTER: {
                if (currEff.constant > 0.0) { // if smaller is better, and this is an increase, it's not compression safe
                    if (Globals::globalVerbosity & 16384) {
                        cout << "Has undesirable increase effect on " << *(RPGBuilder::getPNE(currEff.fluentIndex)) << endl;
                    }
                    return false;
                }
                            

                break;
            }
            case NumericAnalysis::E_BIGGERISBETTER: {
                if (currEff.constant < 0.0) { // if bigger is better, and this is a decrease, it's not compression safe
                    return false;
                    if (Globals::globalVerbosity & 16384) {
                        cout << "Has undesirable decrease effect on " << *(RPGBuilder::getPNE(currEff.fluentIndex)) << endl;
                    }
                    
                }                
                break;
            }
        }
        
    }
    // if nothing tripped the return falses above, then everything must be compression safe
    return true;

}
void TemporalAnalysis::markCompressionSafetyConditions(const int & actID, const list<int> & effects,
                                                       vector<set<int> > & actionsDependingOnVarBeingCompressionSafe)
{
    list<int>::const_iterator effItr = effects.begin();
    const list<int>::const_iterator effEnd = effects.end();
    
    for (; effItr != effEnd; ++effItr) {
        
        const RPGBuilder::RPGNumericEffect & currEff = RPGBuilder::getNumericEff()[*effItr];
        
        actionsDependingOnVarBeingCompressionSafe[currEff.fluentIndex].insert(actID);
    }
    
}

void TemporalAnalysis::markAffectedVariablesAsNotCompressionSafe(const list<int> & effects,
                                                                 vector<bool> & allInteractionWithVarIsCompressionSafe,
                                                                 set<int> & newlyUnsafe)
{
    
    list<int>::const_iterator effItr = effects.begin();
    const list<int>::const_iterator effEnd = effects.end();
    
    for (; effItr != effEnd; ++effItr) {
        
        const RPGBuilder::RPGNumericEffect & currEff = RPGBuilder::getNumericEff()[*effItr];
        if (allInteractionWithVarIsCompressionSafe[currEff.fluentIndex]) {
            allInteractionWithVarIsCompressionSafe[currEff.fluentIndex] = false;
            newlyUnsafe.insert(currEff.fluentIndex);
        }
    }
    
}
                                                   
#endif

vector<bool> TemporalAnalysis::startEndSkip;

void TemporalAnalysis::findCompressionSafeActions()
{

    const int actCount = RPGBuilder::instantiatedOps.size();
    const int pneCount = RPGBuilder::pnes.size();

    startEndSkip.resize(actCount, false);

    if (!RPGBuilder::doSkipAnalysis) return;

    int compressionSafeActionCount = 0;
    int nonRogueCount = 0;
    
    LiteralSet goalsAsASet;
    LiteralSet negativeGoalsAsASet;
    
    goalsAsASet.insert(RPGBuilder::literalGoals.begin(), RPGBuilder::literalGoals.end());

    LiteralSet okayToDeleteAtEnd;
    LiteralSet okayToAddAtEnd;
    
    recogniseHoldThroughoutDeleteAtEndIdiom(okayToDeleteAtEnd);
    
    #ifdef POPF3ANALYSIS
    vector<bool> allInteractionWithVarIsCompressionSafe(pneCount);
    
    for (int v = 0; v < pneCount; ++v) {
        allInteractionWithVarIsCompressionSafe[v] = (    (NumericAnalysis::getDataOnWhichVariablesHaveOrderIndependentEffects()[v] == NumericAnalysis::E_ORDER_INDEPENDENT)
                                                      && NumericAnalysis::getWhichVariablesAreOnlyInStartPreconditions()[v]);
    }
    
    vector<set<int> > actionsDependingOnVarBeingCompressionSafe(pneCount);
    
    #endif
    
    for (int i = 0; i < actCount; ++i) {

        if (!RPGBuilder::rogueActions[i]) {
            if (RPGBuilder::getRPGDEs(i).empty()) continue;

            ++nonRogueCount;

            if (Globals::globalVerbosity & 16384) {
                #ifdef POPF3ANALYSIS
                #ifdef TOTALORDERSTATES
                startEndSkip[i] = RPGBuilder::actionsToRPGNumericEndEffects[i].empty();
                #else
                startEndSkip[i] = endNumericEffectsAreCompressionSafe(RPGBuilder::actionsToRPGNumericEndEffects[i],
                                                                      allInteractionWithVarIsCompressionSafe);
                #endif
                #else
                startEndSkip[i] = RPGBuilder::actionsToRPGNumericEndEffects[i].empty();
                #endif
                
                if (!startEndSkip[i]) {
                    cout << "Action " << *(RPGBuilder::getInstantiatedOp(i)) << " cannot be compression safe - has unsafe end numeric effects\n";
                } else {
                    
                    startEndSkip[i] = RPGBuilder::actionsToRPGNumericInvariants[i].empty() && RPGBuilder::actionsToRPGNumericEndPreconditions[i].empty() && !RPGBuilder::linearDiscretisation[i];
                                  
                    if (!startEndSkip[i]) {
                        cout << "Action " << *(RPGBuilder::getInstantiatedOp(i)) << " cannot be compression safe - has non-start numerics\n";
                    } else {
                        startEndSkip[i] =      firstIsSubsumedBySecond(RPGBuilder::actionsToEndPreconditions[i], RPGBuilder::actionsToInvariants[i])
                                            && firstIsSubsumedBySecond(RPGBuilder::actionsToEndNegativePreconditions[i], RPGBuilder::actionsToNegativeInvariants[i]);
                        if (!startEndSkip[i]) {
                            cout << "Action " << *(RPGBuilder::getInstantiatedOp(i)) << " cannot be compression safe - end pres aren't subsumed by invariants\n";
                        } else {
                            startEndSkip[i] = noOverlap(RPGBuilder::actionsToEndNegativeEffects[i], RPGBuilder::preconditionsToActions, goalsAsASet, okayToDeleteAtEnd);
                            if (!startEndSkip[i]) {
                                cout << "Action " << *(RPGBuilder::getInstantiatedOp(i)) << " cannot be compression safe - end delete effects intersect with needed preconditions\n";
                            } else {
                                startEndSkip[i] = noOverlap(RPGBuilder::actionsToEndEffects[i], RPGBuilder::negativePreconditionsToActions, negativeGoalsAsASet, okayToAddAtEnd);
                                if (!startEndSkip[i]) {
                                    cout << "Action " << *(RPGBuilder::getInstantiatedOp(i)) << " cannot be compression safe - end add effects intersect with needed negative preconditions\n";
                                }
                            }
                        }
                    }
                }
                
            } else {
                #ifdef POPF3ANALYSIS
                #ifdef TOTALORDERSTATES
                startEndSkip[i] = RPGBuilder::actionsToRPGNumericEndEffects[i].empty()
                #else
                startEndSkip[i] =     endNumericEffectsAreCompressionSafe(RPGBuilder::actionsToRPGNumericEndEffects[i], allInteractionWithVarIsCompressionSafe)
                #endif
                #else
                startEndSkip[i] =     RPGBuilder::actionsToRPGNumericEndEffects[i].empty()
                #endif
                                      && RPGBuilder::actionsToRPGNumericInvariants[i].empty()
                                      && RPGBuilder::actionsToRPGNumericEndPreconditions[i].empty() && !RPGBuilder::linearDiscretisation[i]
                                      && firstIsSubsumedBySecond(RPGBuilder::actionsToEndPreconditions[i], RPGBuilder::actionsToInvariants[i])
                                      && firstIsSubsumedBySecond(RPGBuilder::actionsToEndNegativePreconditions[i], RPGBuilder::actionsToNegativeInvariants[i])
                                                                            && noOverlap(RPGBuilder::actionsToEndNegativeEffects[i], RPGBuilder::preconditionsToActions, goalsAsASet, okayToDeleteAtEnd)
                                      && noOverlap(RPGBuilder::actionsToEndEffects[i], RPGBuilder::negativePreconditionsToActions, negativeGoalsAsASet, okayToAddAtEnd);
            }


            if (startEndSkip[i]) {
                bool ddes = false;
                if (RPGBuilder::fixedDurationExpressions[i].empty()) {
                    // check for duration-dependent preconditions/effects

                    {
                        list<int> & currList = RPGBuilder::actionsToRPGNumericStartPreconditions[i];
                        list<int>::iterator clItr = currList.begin();
                        const list<int>::iterator clEnd = currList.end();

                        for (; clItr != clEnd; ++clItr) {

                            RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::rpgNumericPreconditions[*clItr];
                            if (currPre.LHSVariable < -1) {
                                ddes = true;
                                break;
                            } else if (currPre.LHSVariable >= 2 * pneCount) {
                                RPGBuilder::ArtificialVariable & currAV = RPGBuilder::getArtificialVariable(currPre.LHSVariable);
                                for (int j = 0; j < currAV.size; ++j) {
                                    if (currAV.fluents[j] < -1) {
                                        ddes = true;
                                        break;
                                    }
                                }
                                if (ddes) break;
                            }
                        }

                    }

                    if (!ddes) {
                        list<int> & currList = RPGBuilder::actionsToRPGNumericStartEffects[i];
                        list<int>::iterator clItr = currList.begin();
                        const list<int>::iterator clEnd = currList.end();

                        for (; clItr != clEnd; ++clItr) {

                            RPGBuilder::RPGNumericEffect & currEff = RPGBuilder::rpgNumericEffects[*clItr];

                            for (int j = 0; j < currEff.size; ++j) {
                                if (currEff.variables[j] < -1) {
                                    ddes = true;
                                    break;
                                }
                            }
                            if (ddes) break;
                        }

                    }
                }

                startEndSkip[i] = !ddes;
                if (startEndSkip[i]) {
                    ++compressionSafeActionCount;
                    if (Globals::globalVerbosity & 16) {
                        cout << *(RPGBuilder::getInstantiatedOp(i)) << " is a candidate for start-end skipping\n";
                    }
                    
                    #ifdef POPF3ANALYSIS
                    markCompressionSafetyConditions(i, RPGBuilder::actionsToRPGNumericEndEffects[i], actionsDependingOnVarBeingCompressionSafe);
                    #endif
                } else {
                    
                    #ifdef POPF3ANALYSIS
                    
                    set<int> newNonCompressionSafeVariables;                    
                    markAffectedVariablesAsNotCompressionSafe(RPGBuilder::actionsToRPGNumericEndEffects[i], allInteractionWithVarIsCompressionSafe, newNonCompressionSafeVariables);
                    
                    while (!newNonCompressionSafeVariables.empty()) {
                        set<int> varsToUpdateFrom;
                        varsToUpdateFrom.swap(newNonCompressionSafeVariables);
                        
                        set<int>::const_iterator vItr = varsToUpdateFrom.begin();
                        const set<int>::const_iterator vEnd = varsToUpdateFrom.end();
                        
                        for (; vItr != vEnd; ++vItr) {
                            set<int>::const_iterator noncsActItr = actionsDependingOnVarBeingCompressionSafe[*vItr].begin();
                            const set<int>::const_iterator noncsActEnd = actionsDependingOnVarBeingCompressionSafe[*vItr].end();
                            
                            int reversed = 0;
                            for (; noncsActItr != noncsActEnd; ++noncsActItr) {
                                if (startEndSkip[*noncsActItr]) {
                                    startEndSkip[*noncsActItr] = false;
                                    ++reversed;
                                    markAffectedVariablesAsNotCompressionSafe(RPGBuilder::actionsToRPGNumericEndEffects[*noncsActItr], allInteractionWithVarIsCompressionSafe, newNonCompressionSafeVariables);
                                }
                            }
                            
                            if (Globals::globalVerbosity & 16) {
                                cout << "Shown that interaction with " << *(RPGBuilder::getPNE(*vItr)) << " is not compression safe, actions affected: " << reversed << endl;
                            }
                            actionsDependingOnVarBeingCompressionSafe[*vItr].clear();
                        }
                    };
                    
                    #endif
                    
                }
            }

        }

    }

    #ifdef POPF3ANALYSIS    
    if (Globals::globalVerbosity & 16) {
        for (int v = 0; v < pneCount; ++v) {
            if (!actionsDependingOnVarBeingCompressionSafe[v].empty()) {
                cout << "Detecting compression-safety of interaction with " << *(RPGBuilder::getPNE(v)) << " has allowed " << actionsDependingOnVarBeingCompressionSafe[v].size() << " action(s) to be compression safe\n";
            }
        }
    }
    #endif
    
    if (nonRogueCount) {
        if (compressionSafeActionCount == nonRogueCount) {
            cout << "All the ground actions in this problem are compression-safe\n";
        } else if (compressionSafeActionCount == 0) {
            cout << "None of the ground temporal actions in this problem have been recognised as compression-safe\n";
        } else {
            const int percentage = (int)((double) compressionSafeActionCount / (double) nonRogueCount * 100.0);
            cout << percentage << "% of the ground temporal actions in this problem are compression-safe\n";
        }
    }
}

map<int, TILTimeline> TemporalAnalysis::timelineOnTILs;
map<int, TIFTimeline> TemporalAnalysis::timelineOnTIFs;    

void TemporalAnalysis::buildTimelinesOnTILs()
{

    const vector<RPGBuilder::FakeTILAction*> & tilVec = RPGBuilder::getTILVec();
    
    if (tilVec.empty()) {
        return;
    }
    
                            
    LiteralSet initialFacts;
    vector<double> initialFluents;
                        
    RPGBuilder::getInitialState(initialFacts, initialFluents);
    
    const int tilCount = tilVec.size();
    
    for (int til = 0; til < tilCount; ++til) {
        int fID;
        
        for (int addDeletePass = 0; addDeletePass < 2; ++addDeletePass) {
            list<Literal*>::const_iterator addItr = (addDeletePass ? tilVec[til]->delEffects.begin() : tilVec[til]->addEffects.begin());
            const list<Literal*>::const_iterator addEnd = (addDeletePass ? tilVec[til]->delEffects.end() : tilVec[til]->addEffects.end());
            
            for (; addItr != addEnd; ++addItr) {
                
                fID = (*addItr)->getStateID();
                
                map<int, TILTimeline>::iterator fItr = timelineOnTILs.find(fID);
                    
                if (fItr == timelineOnTILs.end()) {
                    
                    fItr = timelineOnTILs.insert(make_pair(fID, TILTimeline())).first;

                    fItr->second.firstAdder = fItr->second.end();
                    fItr->second.firstDeletor = fItr->second.end();
                    fItr->second.lastDeletor = fItr->second.end();
                    
                    if (initialFacts.find(*addItr) != initialFacts.end()) {
                        fItr->second.happenings.insert(make_pair(0.0, TILTimeline::AddedOrDeleted(true)));
                    }
                    
                    for (int pass = 0; pass < 2; ++pass) {
                        const list<pair<int, VAL::time_spec> > & affectedBy = (pass ? RPGBuilder::getNegativeEffectsToActions()[fID] : RPGBuilder::getEffectsToActions(fID));
                        
                        list<pair<int, VAL::time_spec> >::const_iterator actItr = affectedBy.begin();
                        const list<pair<int, VAL::time_spec> >::const_iterator actEnd = affectedBy.end();
                        
                        for (; actItr != actEnd; ++actItr) {
                            
                            if (actItr->second != VAL::E_AT) {
                                if (pass) {
                                    fItr->second.onlyEverDeletedByTILs = false;
                                } else {
                                    fItr->second.onlyEverAddedByTILs = false;
                                } 
                            
                            }
                            
                        }
                    }
                }
                
                if (addDeletePass) {                    
                    fItr->second.happenings.insert(make_pair(tilVec[til]->duration, TILTimeline::AddedOrDeleted(false))).first->second.deleted = true;
                } else {
                    fItr->second.happenings.insert(make_pair(tilVec[til]->duration, TILTimeline::AddedOrDeleted(true))).first->second.added = true;
                }
                
            }
                        
        }
        
    }
    
    map<int, TILTimeline>::iterator tilFactItr = timelineOnTILs.begin();
    const map<int, TILTimeline>::iterator tilFactEnd = timelineOnTILs.end();
    
    for (; tilFactItr != tilFactEnd; ++tilFactItr) {
        
        TILTimeline::const_iterator itr = tilFactItr->second.begin();
        const TILTimeline::const_iterator itrEnd = tilFactItr->second.end();
        
        for (; itr != itrEnd; ++itr) {
            
            if (itr->second.added && tilFactItr->second.firstAdder == tilFactItr->second.end()) {
                tilFactItr->second.firstAdder = itr;
            }
            
            if (itr->second.deleted) {
                if (tilFactItr->second.firstDeletor == tilFactItr->second.end()) {
                    tilFactItr->second.firstDeletor = itr;
                }
                
                tilFactItr->second.lastDeletor = itr;
            }
            
        }
    }
}

void TemporalAnalysis::buildTimelinesOnTIFs()
{
    
    const vector<RPGBuilder::FakeTILAction*> & tilVec = RPGBuilder::getTILVec();
    
    if (tilVec.empty()) {
        return;
    }

    const int tilCount = tilVec.size();
    
    for (int til = 0; til < tilCount; ++til) {
        int fID;
            
            
        map<int,double>::const_iterator neItr = tilVec[til]->numEffects.begin();
        const map<int,double>::const_iterator neEnd = tilVec[til]->numEffects.end();
        
        for (; neItr != neEnd; ++neItr) {
            
            fID = neItr->first;
            
            map<int, TIFTimeline>::iterator fItr = timelineOnTIFs.find(fID);
            
            if (fItr == timelineOnTIFs.end()) {                
                fItr = timelineOnTIFs.insert(make_pair(fID, TIFTimeline())).first;            
            }
                        
            fItr->second.happenings.insert(tilVec[til]->duration);                                                                        
        }
                    
    }
        
    if (timelineOnTIFs.empty()) {
        return;
    }
    
    map<int, TIFTimeline>::iterator fItr;
        
    {
        const vector<RPGBuilder::LinearEffects*> & ctsEffs = RPGBuilder::getLinearDiscretisation();    
        const int actCount = ctsEffs.size();
        for (int actID = 0; actID < actCount; ++actID) {
            if (RPGBuilder::rogueActions[actID] == RPGBuilder::OT_INVALID_ACTION) continue;
            if (!ctsEffs[actID]) continue;
            
            const int varCount = ctsEffs[actID]->vars.size();
            
            for (int i = 0; i < varCount; ++i) {
                fItr = timelineOnTIFs.find(ctsEffs[actID]->vars[i]);
                if (fItr != timelineOnTIFs.end()) {
                    fItr->second.onlyEverChangedByTIFs = false;
                }
            }                        
        }
                            
    }
            
    const vector<RPGBuilder::RPGNumericEffect> & numericEffs = RPGBuilder::getNumericEff();
    
    const int effCount = numericEffs.size();
    
    for (int effID = 0; effID < effCount; ++effID) {
        const RPGBuilder::RPGNumericEffect & currEff = numericEffs[effID];
        
        fItr = timelineOnTIFs.find(currEff.fluentIndex);
        if (fItr != timelineOnTIFs.end()) {
            fItr->second.onlyEverChangedByTIFs = false;
        }
    }
                
}

double TemporalAnalysis::upperBoundOnFactsIfNextTILIsAtTime(const list<Literal*> & facts,
                                                            const bool & epsilonSeparation,
                                                            const double & nextTILTimestamp)
{
    
    if (facts.empty()) {
        return DBL_MAX;
    }
    
    double toReturn = DBL_MAX;
    
    list<Literal*>::const_iterator preItr = facts.begin();
    const list<Literal*>::const_iterator preEnd = facts.end();
    
    for (; preItr != preEnd; ++preItr) {
        
        const map<int, TILTimeline>::iterator fItr = timelineOnTILs.find((*preItr)->getStateID());
        
        if (fItr == timelineOnTILs.end()) {
            continue;
        }
        
        const TILTimeline::const_iterator nextDeletor = fItr->second.firstDeletorAtOrAfterTime(nextTILTimestamp);
        if (nextDeletor != fItr->second.end()) {
            if (nextDeletor->first < toReturn) {
                toReturn = nextDeletor->first;
            }
        }
    }
    
    if (toReturn != DBL_MAX && epsilonSeparation) {
        toReturn -= EPSILON;
    }
    
    return toReturn;
    
}

double TemporalAnalysis::upperBoundOnVariableIfNextTILIsAtTime(const int & variable,
                                                               const bool & epsilonSeparation,
                                                               const double & nextTILTimestamp)
{
    if (variable < 0) {
        return DBL_MAX;
    }
    
    assert(variable < RPGBuilder::getPNECount());
    
    const map<int, TIFTimeline>::iterator fItr = timelineOnTIFs.find(variable);
    
    if (fItr == timelineOnTIFs.end()) {
        return DBL_MAX;
    }

    const TIFTimeline::const_iterator nextChange = fItr->second.changeAtOrAfterTime(nextTILTimestamp);
    
    if (nextChange == fItr->second.end()) {
        return DBL_MAX;
    }
    
    if (epsilonSeparation) {
        return (*nextChange - EPSILON);
    } else {
        return *nextChange;
    }
            
        
}
    
double TemporalAnalysis::upperBoundOnNumericPreconditionsIfNextTILIsAtTime(const list<int> & pres,
                                                                           const bool & epsilonSeparation,
                                                                           const double & nextTILTimestamp)
{

    if (pres.empty()) {
        return DBL_MAX;
    }
            
    static const int pneCount = RPGBuilder::getPNECount();
    
    double toReturn = DBL_MAX;
    
    list<int>::const_iterator pItr = pres.begin();
    const list<int>::const_iterator pEnd = pres.end();
    
    double localResult;
    
    for (; pItr != pEnd; ++pItr) {
        
        const RPGBuilder::RPGNumericPrecondition & currPre = RPGBuilder::getNumericPreTable()[*pItr];
        
        if (currPre.LHSVariable < pneCount) {
            localResult = upperBoundOnVariableIfNextTILIsAtTime(currPre.LHSVariable, epsilonSeparation, nextTILTimestamp);
            if (localResult < toReturn) {
                toReturn = localResult;
            }
        } else if (currPre.LHSVariable < 2 * pneCount) {
            localResult = upperBoundOnVariableIfNextTILIsAtTime(currPre.LHSVariable - pneCount, epsilonSeparation, nextTILTimestamp);
            if (localResult < toReturn) {
                toReturn = localResult;
            }
        } else {
            const RPGBuilder::ArtificialVariable & currAV = RPGBuilder::getArtificialVariable(currPre.LHSVariable);
            
            for (int s = 0; s < currAV.size; ++s) {
                if (currAV.fluents[s] < pneCount) {
                    localResult = upperBoundOnVariableIfNextTILIsAtTime(currAV.fluents[s], epsilonSeparation, nextTILTimestamp);
                    if (localResult < toReturn) {
                        toReturn = localResult;
                    }
                } else {
                    localResult = upperBoundOnVariableIfNextTILIsAtTime(currAV.fluents[s] - pneCount, epsilonSeparation, nextTILTimestamp);
                    if (localResult < toReturn) {
                        toReturn = localResult;
                    }
                }
            }
        }
        
    }
        
    // epsilon separation is dealt with in upperBoundOnVariableIfNextTILIsAtTime
    
    return toReturn;
    
}


    
double TemporalAnalysis::upperBoundOnActionIfNextTILIs(const ActionSegment & theAction, const int & nextTIL)
{

    if (nextTIL >= RPGBuilder::getTILVec().size()) {
        return DBL_MAX;
    }
    
    if (theAction.second == VAL::E_AT) {
        return DBL_MAX;
    }
    
    double toReturn = DBL_MAX;
    
    const double nextTILTimestamp = RPGBuilder::getTILVec()[nextTIL]->duration;
    
    double localResult;
    if (theAction.second == VAL::E_AT_START) {
        localResult = upperBoundOnFactsIfNextTILIsAtTime(RPGBuilder::getProcessedStartPropositionalPreconditions()[theAction.first->getID()],
                                                         true, nextTILTimestamp);
        
        if (localResult < toReturn) {
            toReturn = localResult;
        }
        
        if (!timelineOnTIFs.empty()) {        
            localResult = upperBoundOnNumericPreconditionsIfNextTILIsAtTime(RPGBuilder::getProcessedStartRPGNumericPreconditions()[theAction.first->getID()],
                                                                            true, nextTILTimestamp);
            
            if (localResult < toReturn) {
                toReturn = localResult;
            }
        }
                
    } else {
        
        localResult = upperBoundOnFactsIfNextTILIsAtTime(RPGBuilder::getInvariantPropositionalPreconditions()[theAction.first->getID()],
                                                         false, nextTILTimestamp);
        
        if (localResult < toReturn) {
            toReturn = localResult;
        }

        localResult = upperBoundOnFactsIfNextTILIsAtTime(RPGBuilder::getEndPropositionalPreconditions()[theAction.first->getID()],
                                                         true, nextTILTimestamp);
        
        if (localResult < toReturn) {
            toReturn = localResult;
        }

                
        if (!timelineOnTIFs.empty()) {
                
            localResult = upperBoundOnNumericPreconditionsIfNextTILIsAtTime(RPGBuilder::getInvariantNumerics()[theAction.first->getID()],
                                                                            false, nextTILTimestamp);
            
            if (localResult < toReturn) {
                toReturn = localResult;
            }

            localResult = upperBoundOnNumericPreconditionsIfNextTILIsAtTime(RPGBuilder::getEndPreNumerics()[theAction.first->getID()],
                                                                            true, nextTILTimestamp);
            
            if (localResult < toReturn) {
                toReturn = localResult;
            }
                    
        }                
    }

    return toReturn;
        
}

bool TemporalAnalysis::futureTILsMightInterfereWithGoals(const int & nextTIL)
{

    static const bool localDebug = false;
    
    if (nextTIL >= RPGBuilder::getTILVec().size()) {
        // No TILs left
        if (localDebug) {
            cout << "No TILs remaining (" << nextTIL << " vs " << RPGBuilder::getTILVec().size() << "), so future TILs are not going to interfere with the goals\n";
        }
        return false;
    }
    
    static bool builtNumericGoalList = false;
    static list<int> numericGoalList;
    
    if (!builtNumericGoalList) {
        
        builtNumericGoalList = true;
        
        const list<pair<int,int> > & masterNumericGoals = RPGBuilder::getNumericRPGGoals();
        list<pair<int,int> >::const_iterator gItr = masterNumericGoals.begin();
        const list<pair<int,int> >::const_iterator gEnd = masterNumericGoals.end();
        
        for (; gItr != gEnd; ++gItr) {
            if (gItr->first != -1) {
                numericGoalList.push_back(gItr->first);
            }
            if (gItr->second != -1) {
                numericGoalList.push_back(gItr->second);
            }
        }
    }
    
    const double nextTILTimestamp = RPGBuilder::getTILVec()[nextTIL]->duration;
    
    if (!timelineOnTIFs.empty()) {
    
        if (localDebug) {
            cout << "Checking whether " << numericGoalList.size() << " goals might be altered by timed initial fluents\n";
        }
        if (upperBoundOnNumericPreconditionsIfNextTILIsAtTime(numericGoalList, false, nextTILTimestamp) != DBL_MAX) {
            // there are future TIFs that could affect the truth value of one or more goal constraints
            if (localDebug) {
                cout << "Future TILs might interfere with numeric goals, returning true\n";
            }
            return true;
        }
    }
    
    if (!timelineOnTILs.empty()) {   
        // for the goal facts itself, it's a bit different to the upper bound for actions case:
        // we're okay if a TIL deletes the goal, so long as it's later added again
        
        const list<Literal*> & goals = RPGBuilder::getLiteralGoals();
                    
        list<Literal*>::const_iterator gItr = goals.begin();
        const list<Literal*>::const_iterator gEnd = goals.end();
        
        for (; gItr != gEnd; ++gItr) {
            const map<int, TILTimeline>::iterator fItr = timelineOnTILs.find((*gItr)->getStateID());
            
            if (fItr == timelineOnTILs.end()) {
                // fact isn't affected by TILs
                if (localDebug) {
                    cout << "- " << *(RPGBuilder::getLiteral(fItr->first)) << " never affected by TILs\n";
                }
                continue;
            }
            
            TILTimeline::const_iterator nextDeletor = fItr->second.firstDeletorAtOrAfterTime(nextTILTimestamp);
            
            if (nextDeletor == fItr->second.end()) {
                if (localDebug) {
                    cout << "- " << *(RPGBuilder::getLiteral(fItr->first)) << " has no future TIL deletes\n";
                }
                // no more delete effects by TILs are due to act on this fact
                continue;
            }
            
            bool finishesPositive = nextDeletor->second.added;
            
            const TILTimeline::const_iterator timelineEnd = fItr->second.end();
            
            for (; nextDeletor != timelineEnd; ++nextDeletor) {
                if (nextDeletor->second.deleted) {
                    // TIL deletes the fact: is now false
                    finishesPositive = false;                    
                }
                
                if (nextDeletor->second.added) {
                    // TIL adds the fact: is true again
                    finishesPositive = true;
                }
            }
            
            if (!finishesPositive) {
                if (localDebug) {
                    cout << "- " << *(RPGBuilder::getLiteral(fItr->first)) << " deleted by a future TIL, returning false\n";
                }
                // at the end of the TIL timeline, the goal was false
                return true;
            } else {
                if (localDebug) {
                    cout << "- " << *(RPGBuilder::getLiteral(fItr->first)) << " eventually added by a TIL again\n";
                }
            }
        }
    }
    
    
    if (localDebug) {
        cout << "All goals are safe w.r.t. future TILs\n";
    }
    return false;
}


}