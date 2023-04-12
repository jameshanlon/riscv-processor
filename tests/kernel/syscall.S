#---------------------------------#
# Basic syscalls                  #
#---------------------------------#

.text

.type exit, @function
.global exit
exit:
  li a1, a0
  li a0, 0
  ecall


.type putchar, @function
.global putchar
putchar:
  li a1, a0
  li a0, 1
  ecall


.type getchar, @function
.global getchar
getchar:
  li a0, 2
  ecall
