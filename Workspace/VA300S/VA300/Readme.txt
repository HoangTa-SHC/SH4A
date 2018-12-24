-------- PROJECT GENERATOR --------
PROJECT NAME :	VA300
PROJECT DIRECTORY :	D:\Usr\saeki\Bionics\VA300\Src\VA300\VA300
CPU SERIES :	SH-4
CPU TYPE :	SH7750R
TOOLCHAIN NAME :	Renesas SuperH RISC engine Standard Toolchain
TOOLCHAIN VERSION :	9.3.2.0
GENERATION FILES :
    D:\Usr\saeki\Bionics\VA300\Src\VA300\VA300\dbsct.c
        Setting of B,R Section
    D:\Usr\saeki\Bionics\VA300\Src\VA300\VA300\typedefine.h
        Aliases of Integer Type
    D:\Usr\saeki\Bionics\VA300\Src\VA300\VA300\sbrk.c
        Program of sbrk
    D:\Usr\saeki\Bionics\VA300\Src\VA300\VA300\iodefine.h
        Definition of I/O Register
    D:\Usr\saeki\Bionics\VA300\Src\VA300\VA300\intprg.src
        Interrupt Program
    D:\Usr\saeki\Bionics\VA300\Src\VA300\VA300\vecttbl.src
        Initialize of Vector Table
    D:\Usr\saeki\Bionics\VA300\Src\VA300\VA300\vect.inc
        Definition of Vector
    D:\Usr\saeki\Bionics\VA300\Src\VA300\VA300\env.inc
        Define Interruput Event Register
    D:\Usr\saeki\Bionics\VA300\Src\VA300\VA300\vhandler.src
        Reset/Interrupt Handler
    D:\Usr\saeki\Bionics\VA300\Src\VA300\VA300\resetprg.c
        Reset Program
    D:\Usr\saeki\Bionics\VA300\Src\VA300\VA300\hwsetup.c
        Hardware Setup file
    D:\Usr\saeki\Bionics\VA300\Src\VA300\VA300\VA300.c
        Main Program
    D:\Usr\saeki\Bionics\VA300\Src\VA300\VA300\sbrk.h
        Header file of sbrk file
    D:\Usr\saeki\Bionics\VA300\Src\VA300\VA300\stacksct.h
        Setting of Stack area
START ADDRESS OF SECTION :
 H'000000800	INTHandler,VECTTBL,INTTBL,IntPRG
 H'000001000	PResetPRG
 H'000002000	P,C,C$BSEC,C$DSEC,D
 H'070000000	B,R
 H'08CFFFBFC	S
 H'0A0000000	RSTHandler

* When the user program is executed,
* the interrupt mask has been masked.
* 
* Program start H'2000.
* RAM start H'70000000.

SELECT TARGET :
    SH-4(SH7750R) Functional Simulator
DATE & TIME : 2012/08/01 14:39:37
