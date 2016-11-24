module interface
  private

  public :: call_crystal

contains
  !xx! top-level routines

  subroutine call_crystal(filename0, nc) bind ( C )
    use struct
    use struct_basic

    USE, INTRINSIC :: ISO_C_BINDING
    implicit none

    character (kind=c_char, len=1), dimension (*), intent (in) :: filename0
    integer (kind=c_int) :: nc

    character(len=:), allocatable :: filename
    type(crystal) :: c

    filename = str_c_to_f(filename0, nc)

    call struct_crystal_input(c, filename, .false., .true., .true.)
    call c%init()
    call c%struct_fill(.true., .true., 0, .false., .false., .true., .false.)

  end subroutine call_crystal

  function str_c_to_f(strc, nchar) result(strf)
    use, intrinsic :: iso_c_binding
    integer(kind=C_INT), intent(in) :: nchar
    character(kind=C_CHAR,len=1), intent(in) :: strc(nchar)
    character(len=:), allocatable :: strf

    integer :: i

    allocate(character(len=nchar-1) :: strf)
    do i = 1, nchar-1
      strf(i:i) = strc(i)
    end do

  endfunction str_c_to_f

end module interface
