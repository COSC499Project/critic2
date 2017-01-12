module gtkglext_wrap
  use iso_c_binding

  enum, bind(c) ! GdkGLConfigMode
     enumerator :: GDK_GL_MODE_RGB         = 0
     enumerator :: GDK_GL_MODE_RGBA        = 0       ! same as RGB
     enumerator :: GDK_GL_MODE_INDEX       = ishftc(1,0)
     enumerator :: GDK_GL_MODE_SINGLE      = 0
     enumerator :: GDK_GL_MODE_DOUBLE      = ishftc(1,1)
     enumerator :: GDK_GL_MODE_STEREO      = ishftc(1,2)
     enumerator :: GDK_GL_MODE_ALPHA       = ishftc(1,3)
     enumerator :: GDK_GL_MODE_DEPTH       = ishftc(1,4)
     enumerator :: GDK_GL_MODE_STENCIL     = ishftc(1,5)
     enumerator :: GDK_GL_MODE_ACCUM       = ishftc(1,6)
     enumerator :: GDK_GL_MODE_MULTISAMPLE = ishftc(1,7)  ! not supported yet
  end enum

  enum, bind(c) ! GdkGLRenderType
     enumerator :: GDK_GL_RGBA_TYPE = z'8014'
     enumerator :: GDK_GL_COLOR_INDEX_TYPE = z'8015'
  end enum

  interface
     ! GdkGLConfig* gdk_gl_config_new_by_mode (GdkGLConfigMode mode);
     type(c_ptr) function gdk_gl_config_new_by_mode(mode) bind(c,name="gdk_gl_config_new_by_mode")
       import
       integer(c_int), value :: mode
     end function gdk_gl_config_new_by_mode

     ! gboolean gdk_gl_drawable_gl_begin(GdkGLDrawable *gldrawable, GdkGLContext *glcontext);
     integer(c_int) function gdk_gl_drawable_gl_begin(gldrawable, glcontext) bind(c,name="gdk_gl_drawable_gl_begin")
       import 
       type(c_ptr), value :: gldrawable, glcontext
     end function gdk_gl_drawable_gl_begin

     ! void gdk_gl_drawable_gl_end (GdkGLDrawable *gldrawable);
     subroutine gdk_gl_drawable_gl_end(gldrawable) bind(c,name="gdk_gl_drawable_gl_end")
       import
       type(c_ptr), value :: gldrawable
     end subroutine gdk_gl_drawable_gl_end

     ! gboolean gdk_gl_drawable_is_double_buffered (GdkGLDrawable *gldrawable);
     integer(c_int) function gdk_gl_drawable_is_double_buffered(gldrawable) bind(c,name="gdk_gl_drawable_is_double_buffered")
       import
       type(c_ptr), value :: gldrawable
     end function gdk_gl_drawable_is_double_buffered

     ! void gdk_gl_drawable_swap_buffers (GdkGLDrawable *gldrawable);
     subroutine gdk_gl_drawable_swap_buffers(gldrawable) bind(c,name="gdk_gl_drawable_swap_buffers")
       import
       type(c_ptr), value :: gldrawable
     end subroutine gdk_gl_drawable_swap_buffers

     ! GdkGLContext* gtk_widget_get_gl_context (GtkWidget *widget);
     type(c_ptr) function gtk_widget_get_gl_context(widget) bind(c,name="gtk_widget_get_gl_context")
       import
       type(c_ptr), value :: widget
     end function gtk_widget_get_gl_context

     ! GdkGLWindow  *gtk_widget_get_gl_window (GtkWidget *widget);
     type(c_ptr) function gtk_widget_get_gl_window(widget) bind(c,name="gtk_widget_get_gl_window")
       import
       type(c_ptr), value :: widget
     end function gtk_widget_get_gl_window

     ! gboolean gtk_widget_set_gl_capability (GtkWidget *widget, GdkGLConfig *glconfig, GdkGLContext *share_list, gboolean direct, int render_type);
     integer(c_int) function gtk_widget_set_gl_capability(widget, glconfig, share_list, direct, render_type) bind(c,name="gtk_widget_set_gl_capability")
       import
       type(c_ptr), value :: widget, glconfig, share_list
       integer(c_int), value :: direct
       integer(c_int), value :: render_type
     end function gtk_widget_set_gl_capability

     ! dummy routine for testing
     integer(c_int) function dummy(widget) bind(c,name="dummy")
       import
       type(c_ptr), value :: widget
     end function dummy

  end interface


end module gtkglext_wrap

module handlers
  use gtkglext_wrap
  use gtk
  use iso_c_binding
  implicit none

contains
  subroutine destroy (widget, gdata) bind(c)
    type(c_ptr), value :: widget, gdata
    call gtk_main_quit ()
  end subroutine destroy

  ! configure is emitted when the window changes size or position
  integer(c_int) function configure(da,event,user_data)
    use gtk_draw_hl
    use opengl_gl
    type(c_ptr), value :: da, event, user_data

    type(c_ptr) :: glcontext, gldrawable
    integer(c_int) :: iok
    type(gtkallocation), target:: alloc
    integer(c_int) :: width, height

    glcontext = gtk_widget_get_gl_context(da)
    gldrawable = gtk_widget_get_gl_window(da)

    call gtk_widget_get_allocation(da,c_loc(alloc))
    width = alloc%width
    height = alloc%height

    iok = gdk_gl_drawable_gl_begin (gldrawable, glcontext)

    call glLoadIdentity()
    CALL glViewport (0_glint, 0_glint, int(width,glsizei), int(height,glsizei))
    call glOrtho(-10._gldouble,10._gldouble,-10._gldouble,10._gldouble,-20050._gldouble,10000._gldouble)
    call glEnable(GL_BLEND)
    call glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
    call glScalef(10._glfloat, 10._glfloat, 10._glfloat)
    call gdk_gl_drawable_gl_end(gldrawable)

    configure = TRUE

  end function configure

  ! expose is run when the window is about to be redrawn
  integer(c_int) function expose(da,event,user_data)
    use opengl_gl
    type(c_ptr), value :: da, event, user_data

    type(c_ptr) :: glcontext, gldrawable
    integer(c_int) :: iok

    glcontext = gtk_widget_get_gl_context(da)
    gldrawable = gtk_widget_get_gl_window(da) ! GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (da);

    iok = gdk_gl_drawable_gl_begin (gldrawable, glcontext)

    call glClear(GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT)
    call glPushMatrix()
  
    call glBegin (GL_LINES)
    call glColor3f (1., 0., 0.)
    call glVertex3f (0., 0., 0.)
    call glVertex3f (1., 0., 0.)
    call glEnd ()

    call glBegin (GL_LINES)
    call glColor3f (0., 1., 0.)
    call glVertex3f (0., 0., 0.)
    call glVertex3f (0., 1., 0.)
    call glEnd ()

    call glBegin (GL_LINES)
    call glColor3f (0., 0., 1.)
    call glVertex3f (0., 0., 0.)
    call glVertex3f (0., 0., 1.)
    call glEnd ()

    call glPopMatrix()

    iok = gdk_gl_drawable_is_double_buffered (gldrawable)
    if (iok == TRUE) then
       call gdk_gl_drawable_swap_buffers (gldrawable)
    else
       call glFlush ()
    end if
    call gdk_gl_drawable_gl_end(gldrawable)

    expose = TRUE
  end function expose

end module handlers

program gtkFortran
  use gtkglext_wrap
  use iso_c_binding
  use gtk
  use handlers
  implicit none

  type(c_ptr) :: window, da, glconfig, menu, root_menu, menu_item
  type(c_ptr) :: menu_bar, vbox
  integer(c_int) :: iok

  call gtk_init () ! gtk_gl_init (&argc, &argv);

  ! create the window and set up some signals for it.
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL)
  call gtk_window_set_default_size(window, 800, 600)
  call gtk_window_set_title(window,"GTK + OpenGL from Fortran."//c_null_char)
  call g_signal_connect(window,"destroy"//c_null_char,c_funloc(destroy))

  ! vbox and main window
  vbox = gtk_vbox_new(FALSE, 0_c_int)
  call gtk_container_add(window, vbox)

  ! menu
  menu = gtk_menu_new()
  root_menu = gtk_menu_item_new_with_label("File"//c_null_char)
  menu_item = gtk_menu_item_new_with_label("Bleh..."//c_null_char)
  call gtk_menu_shell_append(menu, menu_item)
  call gtk_menu_item_set_submenu(root_menu, menu)
  menu_bar = gtk_menu_bar_new()
  call gtk_container_add(window, menu_bar)
  call gtk_menu_shell_append(menu_bar, root_menu)
  call gtk_box_pack_start (vbox, menu_bar, FALSE, TRUE, 0_c_int)

  ! drawing area (from gtkglext)
  da = gtk_drawing_area_new()
  call gtk_container_add(window, da)
  call gtk_widget_set_events(da, GDK_EXPOSURE_MASK)
  call gtk_box_pack_start(vbox, da, TRUE, TRUE, 0_c_int)

  ! prepare GL
  glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGB + GDK_GL_MODE_DEPTH + GDK_GL_MODE_DOUBLE)
  if (.not.c_associated(glconfig)) then
     write (*,*) "glconfig is NULL"
     stop 1
  end if
  iok = gtk_widget_set_gl_capability(da, glconfig, c_null_ptr, TRUE, GDK_GL_RGBA_TYPE)
  if (iok == 0) then
     write (*,*) "failed gtk_widget_set_gl_capability"
     stop 1
  end if

  ! connect events
  call g_signal_connect(da,"configure-event"//c_null_char,c_funloc(configure),c_null_ptr)
  call g_signal_connect(da,"expose-event"//c_null_char,c_funloc(expose),c_null_ptr)

  ! show all and main loop
  call gtk_widget_show_all(window)
  call gtk_main ()

end program gtkFortran
