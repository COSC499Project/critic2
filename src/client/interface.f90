module interface
  use, intrinsic :: iso_c_binding
  implicit none
  private

  public :: initialize
  public :: call_crystal
  public :: get_positions

contains
  !xx! top-level routines
  subroutine initialize() bind (c,name="initialize")
    use graphics, only: graphics_init
    use spgs, only: spgs_init
    use fields, only: fields_init
    use struct_basic, only: cr
    use config, only: datadir
    use global, only: global_init, fileroot
    use tools_io, only: stdargs, ioinit
    use param, only: param_init
    character(len=:), allocatable :: optv
    character(len=:), allocatable :: ghome
    character(len=:), allocatable :: uroot

    ! initialize parameters
    call param_init()

    ! input/output, arguments (tools_io)
    call ioinit()
    call stdargs(optv,ghome,fileroot)

    ! set default values and initialize the rest of the modules
    call global_init(ghome,datadir)
    call cr%init()
    call fields_init()
    call spgs_init()
    call graphics_init()

  end subroutine initialize

  subroutine call_crystal(filename0, nc) bind(c,name="call_crystal")
    use fields, only: nprops, integ_prop, f, type_grid, itype_fval, itype_lapval,&
       fields_integrable_report
    use grd_atomic, only: grda_init
    use struct, only: struct_crystal_input
    use struct_basic, only: cr
    use tools_io, only: uout, string
    use global, only: refden, gradient_mode, INT_radquad_errprop_default, INT_radquad_errprop
    use autocp, only: init_cplist

    character (kind=c_char, len=1), dimension (*), intent (in) :: filename0
    integer (kind=c_int), value :: nc

    character(len=:), allocatable :: filename

    filename = str_c_to_f(filename0, nc)

    call struct_crystal_input(cr, filename, .true., .true., .true.)
    if (cr%isinit) then
       ! initialize the radial densities
       call grda_init(.true.,.true.,.true.)

       ! header and change refden
       write (uout,'("* Field number ",A," is now REFERENCE."/)') string(0)
       refden = 0
       call init_cplist(.true.)

       ! define second integrable property as the valence charge.
       nprops = max(2,nprops)
       integ_prop(2)%used = .true.
       integ_prop(2)%itype = itype_fval
       integ_prop(2)%fid = 0
       integ_prop(2)%prop_name = "Pop"

       ! define third integrable property as the valence laplacian.
       nprops = max(3,nprops)
       integ_prop(3)%used = .true.
       integ_prop(3)%itype = itype_lapval
       integ_prop(3)%fid = 0
       integ_prop(3)%prop_name = "Lap"

       ! reset defaults for qtree
       if (f(refden)%type == type_grid) then
          gradient_mode = 1
          if (INT_radquad_errprop_default) INT_radquad_errprop = 2
       else
          gradient_mode = 2
          if (INT_radquad_errprop_default) INT_radquad_errprop = 3
       end if

       ! report
       call fields_integrable_report()
    else
       call cr%init()
    end if

  end subroutine call_crystal

  function str_c_to_f(strc, nchar) result(strf)
    integer(kind=C_INT), intent(in), value :: nchar
    character(kind=C_CHAR,len=1), intent(in) :: strc(nchar)
    character(len=:), allocatable :: strf

    integer :: i

    allocate(character(len=nchar) :: strf)
    do i = 1, nchar
      strf(i:i) = strc(i)
    end do

  endfunction str_c_to_f

  subroutine get_positions(n,z,x) bind(c,name="get_positions")
    use struct_basic, only: cr
    integer(c_int), intent(out) :: n
    type(c_ptr), intent(out) :: z
    type(c_ptr), intent(out) :: x
    integer(c_int), allocatable, target, save :: iz(:)
    real(c_double), allocatable, target, save :: ix(:,:)

    integer :: i, j

    n = cr%ncel

    allocate(iz(cr%ncel))
    do i = 1, cr%ncel
       iz(i) = int(cr%at(cr%atcel(i)%idx)%z,C_INT)
    end do

    allocate(ix(cr%ncel,3))
    do i = 1, cr%ncel
       do j = 1, 3
          ix(i,j) = real(cr%atcel(i)%r(j),c_double)
       end do
    end do

    x = c_loc(ix)
    z = c_loc(iz)

  end subroutine get_positions

  !subroutine get_atomic_name(atomName, atomNum) bind (c, name="get_atomic_name")
    !use struct_basic, only: cr
    !implicit none
    !character (kind=c_char, len=1), dimension(10), intent (out) :: atomName
    !integer (kind=c_int), value :: atomNum

    !character(len=:), allocatable, target, save :: iname

    !print*,"asdasda"


    !allocate(iname(size(cr%at(cr%atcel(atomNum)%idx)%name)))

    !print*,iname
    !atomName = c_loc(iname)

  !end subroutine get_atomic_name

  subroutine share_bond(n_atom, connected_atoms) bind (c, name="share_bond")
    use struct_basic, only: cr
    integer (kind=c_int), value :: n_atom
    type(c_ptr), intent(out) :: connected_atoms
    integer(c_int), allocatable, target, save :: iz(:)
    integer :: i

    call cr%find_asterisms()

    allocate(iz(size(cr%nstar(n_atom)%idcon)))
    do i = 1, size(cr%nstar(n_atom)%idcon)
      iz(i) = int(cr%nstar(n_atom)%idcon(i))
    end do

    connected_atoms = c_loc(iz)

  end subroutine share_bond

end module interface
