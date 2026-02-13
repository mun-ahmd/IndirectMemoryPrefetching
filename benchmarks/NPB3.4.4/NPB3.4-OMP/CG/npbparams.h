! CLASS = A
!  
!  
!  This file is generated automatically by the setparams utility.
!  It sets the number of processors and the class of the NPB
!  in this directory. Do not modify it by hand.
!  
        integer            na, nonzer, niter
        double precision   shift, rcond
        parameter(  na=14000, &
     &              nonzer=11, &
     &              niter=15, &
     &              shift=20., &
     &              rcond=1.0d-1 )
        integer, parameter :: kz=4
        logical  convertdouble
        parameter (convertdouble = .false.)
        character compiletime*11
        parameter (compiletime='10 Feb 2026')
        character npbversion*5
        parameter (npbversion='3.4.4')
        character cs1*6
        parameter (cs1='(none)')
        character cs2*5
        parameter (cs2='$(FC)')
        character cs3*6
        parameter (cs3='(none)')
        character cs4*6
        parameter (cs4='(none)')
        character cs5*12
        parameter (cs5='-O3 -fopenmp')
        character cs6*23
        parameter (cs6='$(FFLAGS) $(STATIC_FLG)')
        character cs7*6
        parameter (cs7='randi8')
