;CREATION de la Machine Virtuelle avec initialisation des piles et des pointeurs
;Ici, la pile de Controle et la pile de Donnees sont separees. Le role de chacune est ainsi plus explicites.

(defun make-vm (vm taille-pile)
	(let(
			(init-pile-donnees
				(progn
					(setf (get vm ':size) taille-pile)
					(make-array (list taille-pile))
				)
			)
			(init-pile-controle
				(progn
					(setf (get vm ':size) taille-pile)
					(make-array (list taille-pile))
				)
			)
		)

		(setf (get vm ':pile-controle) init-pile-controle)
		(setf (get vm ':pile-donnees) init-pile-donnees)

		;On stocke la taille de la pile de controle
		(setf (aref (get vm ':pile-controle) 1) taille-pile))

		;On stocke la taille de la pile de donnees
		(setf (aref (get vm ':pile-controle) O) taille-pile))

		;On stocke le sommet de la pile (SP Stack Pointer) SP >= base de pile car montante
		(setf (aref (get vm ':pile-controle) 2) 0))

		;On stocke le compteur de programme (PC Pointer Counter)
		(setf (aref (get vm ':pile-controle) 3) -1))

		;On stocke le pointeur de cadre (FP Frame Pointer) Définit les blocs de pile pour la structurer et faciliter les acces
		(setf (aref (get vm ':pile-controle) 4) 0))

		;On stocke la base de pile (BP Base Pointer) 
		(setf (aref (get vm ':pile-controle) 5) 0))
	)
)

(defun interpret-vm (vm expr)
	(cond ((consp expr) 
	    (if (equal (car expr) ':CONST) ;Empile le litteral
	        ((setf (aref (get vm ':pile-donnees) (aref (get vm ':pile-controle) 2)) (cadr expr)) ;On empile le lit dans la pile de donnees en récupérant la position du SP
	        (setf (aref (get vm ':pile-controle) 2) (+ (aref (get vm ':pile-controle) 2) 1))) ;On incrémente la position du SP
	    )
	  
	    (if (equal (car expr) ':VAR) ;Empile le n ieme variable du bloc de pile courant
	     	(setf (aref (get vm ':pile-donnees) (aref (get vm ':pile-controle) 2)) (aref (get vm ':pile-donnees) (cadr expr))) ;On empile la valeur de la n ieme variable
	    	(setf (aref (get vm ':pile-controle) 2) (+ (aref (get vm ':pile-controle) 2) 1)) ;On incrémente la position du SP
	    )
	    
	    (if (equal (car expr) ':SET-VAR) ;Affecte la valeur depilee a la n ieme variable du bloc de pile courant
	     	(setf (aref (get vm ':pile-donnees) (cadr expr)) (aref (get vm ':pile-donnees) (aref (get vm ':pile-controle) 2))) ;On récupère la valeur du sommet de pile SP et on la met à la n ieme position dans la pile de donnees
	    )
	  
	    (if (equal (car expr) ':STACK) ;Reserve un bloc de pile de taille n
	     	(setf (aref (get vm ':pile-controle) 4) (+ (aref (get vm ':pile-controle) 4) (cadr expr))) ;On réserve un bloc de pile de taille n en faisant FP+n
	     	)
	  
        (if (equal (car expr) ':CALL) ;Appelle la fonction f qui depile ses parametres et empile son resultat
      		)
  
        (if (equal (car expr) ':RTN) ;Retourne d'un appel precedent
     		)

        (if (equal (car expr) ':SKIP) ;Saute n instructions
     		(setf (aref (get nom-vm ':controlStack) 3) (+ (aref (get nom-vm ':controlStack) 3) (cadr expr)))) ;On incrémente le PC de n instructions
  
        (if (equal (car expr) ':SKIPNIL) ;Saute n instructions si la valeur depilee est NIL
      		(if (equal (aref (get vm ':pile-donnees) (aref (get vm ':pile-controle) 2)) nil) ;Si le sommet de pile est NIL, on saute de n insctructions
      			(setf (aref (get nom-vm ':pile-controle) 3) (+ (aref (get nom-vm ':pile-controle) 3) (cadr expr))))
      	)
	  
	    (if (equal (car expr) ':SKIPTRUE) ;Saute n instructions si la valeur depilee n'est pas NIL
      		(if (not (equal (aref (get vm ':pile-donnees) (aref (get vm ':pile-controle) 2)) nil)) ;Si le sommet de pile n'est pas NIL, on saute de n insctructions
      			(setf (aref (get nom-vm ':pile-controle) 3) (+ (aref (get nom-vm ':pile-controle) 3) (cadr expr))))
      	)	

	    (if (equal (car expr) ':LOAD) ;Empile le contenu du mot mémoire d'adresse n
	     	) ; ??   
	  
	    (if (equal (car expr) ':STORE) ;Affecte au mot memoire d'adresse n la valeur depilee
	     	) ; ??
    )
)
