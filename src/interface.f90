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

  subroutine call_crystal(filename) bind ( C, name="call_crystal" )

    use iso_c_binding, only: C_CHAR, c_null_char
    implicit none

    character (kind=c_char, len=1), dimension (40), intent (in) :: filename

    if (cr%isinit) call clean_structure()
    ! read the crystal environment
    call struct_crystal_input(cr,filename,.false.,.true.,.true.)
    if (cr%isinit) then
       ! initialize the radial densities
       call grda_init(.true.,.true.,.true.)
       ! set the promolecular density as reference
       call set_reference(0)
    else
       call cr%init()
    end if

  end subroutine call_crystal
