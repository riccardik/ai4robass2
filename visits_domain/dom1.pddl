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
)

(:durative-action goto_region
		:parameters (?v - robot ?from ?to - region)
		:duration (= ?duration 100)
		:condition (and (at start (robot_in ?v ?from)))
		:effect (and 
			(at start (not (robot_in ?v ?from))) 
			(at start (increase (triggered ?from ?to) 1))   	 ;use trigger to tell to the external module which is the start and the end position
			(at end (robot_in ?v ?to)) 
			(at end (assign (triggered ?from ?to) 0)) 			  ;remove the trigger from start and goal
			(at end (visited ?to)) 	
			;(at end (decrease (act-cost) 1))
			;(at end (increase (dummy) 1))
			;(at end(assign (act-cost) (+ (act-cost) 1)))
			(at end (increase (act-cost) dummy))
			;(at end (increase (dummy) 1))
			;(at end (increase (dummy) (act-cost)))
			(at end (increase (totalcost) dummy))
		)
)


;;(:durative-action localize
;; ...................
;;)



)

