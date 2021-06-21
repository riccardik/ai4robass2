(define (domain localization)

(:requirements :typing :durative-actions :numeric-fluents :negative-preconditions :action-costs :conditional-effects :equality :fluents )


(:types 	robot region 
)

(:predicates
		(robot_in ?v - robot ?r - region) 
		(visited ?r - region )
	      
)

(:functions 
		(act-cost) 
		(triggered ?from ?to - region) 
		(dummy)
		(totalcost)
		(actcost)
		(distance ?from ?to - region)
		(ciao)
		(computed ?from ?to - region)
)

(:durative-action goto_region
		:parameters (?v - robot ?from ?to - region)
		:duration (= ?duration (distance ?from ?to))
		:condition (and (at start (robot_in ?v ?from))
						;(over all (not (?from ?to)))
						;(at start (> (distance ?from ?to) 0))
					;(at start (= ciao 1))
					(at start (= (computed ?from ?to) 1))
						)
		:effect (and 
			(at start (not (robot_in ?v ?from))) 
			;(at start (increase (triggered ?from ?to) 1))   	 ;use trigger to tell to the external module which is the start and the end position
			(at end (robot_in ?v ?to)) 
			;(at end (assign (triggered ?from ?to) 0)) 			  ;remove the trigger from start and goal
			(at end (visited ?to)) 	
			;(at end (decrease (act-cost) 1))
			;(at end (increase (dummy) 1))
			;(at end(assign (act-cost) (+ (act-cost) 1)))
			;(at end (increase (act-cost) dummy))
			;(at end (increase (dummy) 1))
			;(at end (increase (dummy) (act-cost)))
			;(at start (assign (distance ?from ?to) (actcost)))
			;(at end (increase (totalcost) (distance ?from ?to)))
			;(at start (increase (triggered ?from ?to) 1))
			;(at end (assign (triggered ?from ?to) 0)) 

			;(at start (increase (distance ?from ?to) (actcost)))
			
			(at end (increase (totalcost) (distance ?from ?to)))

			;(at end (assign (distance ?from ?to) -1))
			;(at end(assign (ciao)0))
		)
)



(:durative-action localize
		:parameters (?v - robot ?from ?to - region)
		:duration (= ?duration 1)
		:condition (and (at start (robot_in ?v ?from))
						;(at start (= (distance ?from ?to) -1))
						;(over all (not (?from ?to)))
						(at start (= (computed ?from ?to) 0))
		
					)
		:effect (and 
			(at start (increase (triggered ?from ?to) 1))
			;(at start (assign (distance ?from ?to) 0))

			(at end (assign (triggered ?from ?to) 0)) 

			(at end (increase (distance ?from ?to) (actcost)))
			;(at end (increase (totalcost) (actcost)))
			;(at end (increase (totalcost) (distance ?from ?to)))
			;(at end (assign ciao 1))
			(at end (assign (computed ?from ?to) 1))

		)

)



)

