real*8 function function_sum(fsize,fvec) bind (c,name="function_sum")

    integer fsize,i
    real*8 fvec(fsize)
    real*8 sum

    function_sum=0.0

    do i=1,fsize
      function_sum=function_sum+fvec(i)
    end do

    return

end

subroutine subroutine_sum(fsize,fvec,sum) bind (c,name="subroutine_sum")

    integer fsize,i
    real*8 fvec(fsize)
    real*8 sum

    sum=0.0

    do i=1,fsize
      sum=sum+fvec(i)
    end do

end
