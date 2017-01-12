module imgui_implglut_wrapper
  use iso_c_binding
  implicit none
  private

  ! bool ImGui_ImplGLUT_Init();
  public ImGui_ImplGLUT_Init
  interface
     logical(C_BOOL) function ImGui_ImplGLUT_Init() bind(c,name="ImGui_ImplGLUT_Init")
       import
     end function  ImGui_ImplGLUT_Init
  end interface

  ! void ImGui_ImplGLUT_NewFrame(int w, int h);
  public ImGui_ImplGLUT_NewFrame
  interface
     subroutine ImGui_ImplGLUT_NewFrame(w, h) bind(c,name="ImGui_ImplGLUT_NewFrame")
       import
       integer(C_INT), intent(in), value :: w, h
     end subroutine ImGui_ImplGLUT_NewFrame
  end interface

  ! void ImGui_ImplGLUT_Shutdown();
  public ImGui_ImplGLUT_Shutdown
  interface
     subroutine ImGui_ImplGLUT_Shutdown() bind(c,name="ImGui_ImplGLUT_Shutdown")
     end subroutine ImGui_ImplGLUT_Shutdown
  end interface

  ! void igText(const char* fmt)
  public ImGui_Text
  interface ImGui_Text
     module procedure ImGui_Text_fortran
  end interface

  ! void igRender()
  public ImGui_Render
  interface 
     subroutine ImGui_Render() bind(c,name="igRender")
     end subroutine ImGui_Render
  end interface

  ! bool igSliderFloat(const char* label, float* v, float v_min, float v_max, const char* display_format, float power)
  public ImGui_SliderFloat
  interface
     logical(C_BOOL) function ImGui_SliderFloat(label,v,v_min,v_max,display_format,power) bind(c,name="igSliderFloat")
       import
       character(C_CHAR), dimension(*) :: label
       real(C_FLOAT) :: v
       real(C_FLOAT), value :: v_min
       real(C_FLOAT), value :: v_max
       character(C_CHAR), dimension(*) :: display_format
       real(C_FLOAT), value :: power
     end function ImGui_SliderFloat
  end interface

  ! ImGuiIO* igGetIO()
  public ImGui_GetIO
  interface
     type(c_ptr) function ImGui_GetIO() bind(c,name="igGetIO")
       import
     end function ImGui_GetIO
  end interface

  ! private
  interface
     ! void igText(const char* fmt)
     subroutine igText(fmt) bind(c,name="igText")
       import
       character(C_CHAR), dimension(*), intent(in) :: fmt
     end subroutine igText
  end interface
contains

  subroutine ImGui_Text_fortran(str)
    character*(*), intent(in) :: str

    call igText(trim(str) // C_NULL_CHAR)

  end subroutine ImGui_Text_fortran

end module imgui_implglut_wrapper

module global
  use opengl_kinds
  use iso_c_binding
  implicit none
  
  public

  integer(GLint) :: screenWidth = 1280
  integer(GLint) :: screenHeight = 720
  
contains
  
  subroutine drawgui() bind(c)
    use imgui_implglut_wrapper
    implicit none

    real(C_FLOAT) :: f = 0d0
    logical(C_BOOL) :: ok

    call ImGui_ImplGLUT_NewFrame(500, 600)
    call ImGui_Text("Hello, world!")
    ok = ImGui_SliderFloat("float" // C_NULL_CHAR,f,0.0,1.0,"%.3f" // C_NULL_CHAR,1.0) 

    ! if (ImGui::Button("Test Window")) show_test_window ^= 1;
    ! if (ImGui::Button("Another Window")) show_another_window ^= 1;
    ! ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    call ImGui_Render()

  end subroutine drawgui

  ! adapted from f03gl sphere.f90
  subroutine display() bind(c)
    use opengl_gl
    use opengl_glut
    implicit none

    ! clear
    call glclear(GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT)

    ! draw something in the background
    call glColor3f(1.0, 0.0, 0.0)
    call glBegin(GL_TRIANGLES)
    call glVertex3f(-1.0, -1.0, 0.0)
    call glVertex3f(1.0, -1.0, 0.0)
    call glVertex3f(0.0, 1.0, 0.0)
    call glEnd()

    ! draw the gui
    call drawgui()

    ! swap buffers
    call glutSwapBuffers()
    call glutPostRedisplay()

  end subroutine display

  subroutine mouseCallback(button, state, x, y) bind(c)
    use opengl_gl
    use opengl_glut
    use imgui_implglut_wrapper
    integer(GLint), value :: button, state, x, y
    
    interface
       logical (C_BOOL) function mouseEvent(x, y, gdown, gleft, gright) bind(c,name="mouseEvent")
         import C_BOOL, C_INT
         integer (C_INT), value :: x, y
         logical (C_BOOL) :: gdown, gleft, gright
       end function mouseEvent
    end interface

    logical(C_BOOL) :: gdown, gleft, gright

    gdown = (state == GLUT_DOWN)
    gright = (button == GLUT_RIGHT_BUTTON)
    gleft = (button == GLUT_LEFT_BUTTON)

    if (mouseEvent(x,y,gdown,gleft,gright)) then
       call glutPostRedisplay()
    end if

  end subroutine mouseCallback

end module global

program main
  use imgui_implglut_wrapper
  use opengl_glut
  use opengl_gl
  use global
  use iso_c_binding
  implicit none

  integer(GLInt) :: idum
  logical(C_BOOL) :: ok

  ! create window
  call glutInit()
  call glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS)
  call glutInitDisplayMode(GLUT_RGBA+GLUT_DEPTH+GLUT_DOUBLE+GLUT_MULTISAMPLE)
  call glutInitWindowSize(screenWidth, screenHeight)
  call glutInitWindowPosition(200, 200)
  idum = glutcreatewindow('imgui/Fortran example')

  ! call backs
  call glutDisplayFunc(display)
  ! glutReshapeFunc(reshape);
  ! glutKeyboardFunc(keyboardCallback);
  ! glutSpecialFunc(KeyboardSpecial);
  call glutMouseFunc(mouseCallback)
  ! glutMouseWheelFunc(mouseWheel);
  ! glutMotionFunc(mouseDragCallback);
  ! glutPassiveMotionFunc(mouseMoveCallback);

  ! initialization
  call glClearColor(0.447, 0.565, 0.604, 1.0)
  call glClear(GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT)
  ok = ImGui_ImplGLUT_Init()

  ! main loop
  call glutMainLoop()
  
  ! shut down
  call ImGui_ImplGLUT_Shutdown()

end program main

