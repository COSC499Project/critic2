module interface
  use stm
  use xdm
  use ewald
  use hirshfeld
  use qtree
  use bisect
  use integration
  use flux
  use autocp
  use nci
  use rhoplot
  use fields
  use varbas
  use grd_atomic
  use struct
  use struct_basic
  use wfn_private
  use pi_private
  use spgs
  use global
  use config
  use graphics
  use arithmetic
  use types
  use tools
  use tools_io
  use param

  private

  public :: call_crystal

contains
  !xx! top-level routines

  subroutine call_crystal(filename) bind ( C )

    use iso_c_binding, only: C_CHAR, c_null_char
    implicit none

    character (kind=c_char, len=1), dimension (*), intent (in) :: filename

    integer :: lp
    lp = LEN(filename)

  end subroutine call_crystal
end module interface
