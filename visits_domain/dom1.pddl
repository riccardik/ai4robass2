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
		(computed ?from ?to - region)
)

(:durative-action goto_region									;perform the computed path
		:parameters (?v - robot ?from ?to - region)
		:duration (= ?duration (distance ?from ?to))
		:condition (and (at start (robot_in ?v ?from))						
						(at start (= (computed ?from ?to) 1))
					)
		:effect (and 
			(at start (not (robot_in ?v ?from))) 	

			(at end (robot_in ?v ?to)) 			  
			(at end (visited ?to)) 				
			(at end (increase (totalcost) (distance ?from ?to)))

		
		)
)



(:durative-action localize_simulation							;compute a path between the waypoint (the distance)
		:parameters (?v - robot ?from ?to - region)
		:duration (= ?duration 1)
		:condition (and (at start (robot_in ?v ?from))
						(at start (= (computed ?from ?to) 0))		
					)
		:effect (and 
			(at start (increase (triggered ?from ?to) 1)) 		;use trigger to tell to the external module which is the start and the end position			

			(at end (assign (triggered ?from ?to) 0))     		;remove the trigger from start and goal
			(at end (increase (distance ?from ?to) (actcost)))	;compute the actual distance between the waypoints			
			(at end (assign (computed ?from ?to) 1))			;set as computed the distance between start and goal

		)

)



)

