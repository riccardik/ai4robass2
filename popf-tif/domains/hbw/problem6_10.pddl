(define (problem pfile1)
(:domain blocksworld)
(:objects
B0 B1 B2 B3 B4 B5 - block
C0 C1 C2 - column
)
(:init
(inplan)
(arm-empty)

(= (weight-block B0) 5)
(above B0 C0)
(on-column B0)
(= (weight-block B1) 9)
(above B1 C0)
(on B1 B0)
(= (weight-block B2) 7)
(above B2 C0)
(on B2 B1)
(= (weight-block B3) 2)
(above B3 C0)
(on B3 B2)
(= (weight-block B4) 4)
(above B4 C0)
(on B4 B3)
(clear B4)
(= (weight-block B5) 7)
(above B5 C1)
(on-column B5)
(clear B5)
(= (weight-column C0) 27)
(= (maxheight C0) 10)
(= (height-column C0) 9)
(= (weight-column C1) 7)
(= (maxheight C1) 10)
(= (height-column C1) 9)
(= (weight-column C2) 0)
(= (maxheight C2) 10)
(= (height-column C2) 9)
(column-empty C2)
)
(:goal
(and (on-column B0)
(on-column B1)
(on B2 B1)
(on B3 B2)
(on B4 B0)
(on B5 B3)
	    (<= (height-column C0) (maxheight C0))
	    (>= (height-column C0) 0)
	    (<= (height-column C1) (maxheight C1))	
	    (>= (height-column C1) 0)
	    (<= (height-column C2) (maxheight C2))
	    (>= (height-column C2) 0)
)))
