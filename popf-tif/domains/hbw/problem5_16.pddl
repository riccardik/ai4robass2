(define (problem pfile1)
(:domain blocksworld)
(:objects
B0 B1 B2 B3 B4 - block
C0 C1 C2 - column
)
(:init
(arm-empty)

(= (weight-block B0) 5)
(above B0 C0)
(on-column B0)
(= (weight-block B1) 9)
(above B1 C1)
(on-column B1)
(= (weight-block B2) 7)
(above B2 C0)
(on B2 B0)
(clear B2)
(= (weight-block B3) 2)
(above B3 C1)
(on B3 B1)
(clear B3)
(= (weight-block B4) 4)
(above B4 C2)
(on-column B4)
(clear B4)
(= (weight-column C0) 12)
(= (maxheight C0) 10)
(= (height-column C0) 9)
(= (weight-column C1) 11)
(= (maxheight C1) 10)
(= (height-column C1) 9)
(= (weight-column C2) 4)
(= (maxheight C2) 10)
(= (height-column C2) 9)
)
(:goal
(and (on-column B0)
(on B1 B0)
(on-column B2)
(on B3 B1)
(on B4 B2)
	    (<= (height-column C0) (maxheight C0))
	    (>= (height-column C0) 0)
	    (<= (height-column C1) (maxheight C1))	
	    (>= (height-column C1) 0)
	    (<= (height-column C2) (maxheight C2))
	    (>= (height-column C2) 0)
)))
