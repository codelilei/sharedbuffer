## SharedBuffer

A buffer wrapper to save the best buffer for each object during a time sequence in a shared way.


## Example
See comments in the source code for further details.

#### notation
    P              primary, the input buffer for every iteration
    *  *  *  *  *  basic memory buffer unit
       A  B  C  D  4 objects for example

#### time1
    { 1, 1, 1, 1 },
    P 
    1  1  1  1  1
       A  B  C  D

#### time2
    { 1, 2, 2, 1 },
    P
    2  1  1  1  1
    BC A        D

#### time3
    { 3, 2, 2, 1 },
          P
    2  1  3  1  1
    BC    A     D

#### time4
    { 3, 4, 2, 4 },
       P
    2  4  3  1  1
    C  BD A

#### time5
    { 3, 4, 2, 5 }
             P
    2  4  3  5  1
    C  B  A  D


