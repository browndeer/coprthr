; ModuleID = '__vcore_rt.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128"
target triple = "x86_64-unknown-linux-gnu"

%struct.vc_data = type { i32, [8 x i64]*, [8 x i64]*, [8 x i64]*, %struct.work_struct*, [3 x i64] }
%struct.work_struct = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

define void @barrier(i32 %flags) nounwind {
entry:
  %flags_addr = alloca i32                        ; <i32*> [#uses=2]
  %data = alloca %struct.vc_data*                 ; <%struct.vc_data**> [#uses=3]
  %"alloca point" = bitcast i32 0 to i32          ; <i32> [#uses=0]
  store i32 %flags, i32* %flags_addr
  call void asm sideeffect "movq %rbp,%r14\0A\09andq $$-16384,%r14\0A\09movq %r14,$0\0A", "=*m,~{dirflag},~{fpsr},~{flags},~{r14}"(%struct.vc_data** %data) nounwind
  %0 = load %struct.vc_data** %data, align 8      ; <%struct.vc_data*> [#uses=1]
  %1 = getelementptr inbounds %struct.vc_data* %0, i32 0, i32 2 ; <[8 x i64]**> [#uses=1]
  %2 = load [8 x i64]** %1, align 8               ; <[8 x i64]*> [#uses=1]
  %3 = bitcast [8 x i64]* %2 to i64*              ; <i64*> [#uses=1]
  %4 = call i32 (...)* @fast_setjmp(i64* %3) nounwind ; <i32> [#uses=1]
  %5 = icmp eq i32 %4, 0                          ; <i1> [#uses=1]
  br i1 %5, label %bb, label %bb1

bb:                                               ; preds = %entry
  %6 = load %struct.vc_data** %data, align 8      ; <%struct.vc_data*> [#uses=1]
  %7 = getelementptr inbounds %struct.vc_data* %6, i32 0, i32 3 ; <[8 x i64]**> [#uses=1]
  %8 = load [8 x i64]** %7, align 8               ; <[8 x i64]*> [#uses=1]
  %9 = bitcast [8 x i64]* %8 to i64*              ; <i64*> [#uses=1]
  %10 = load i32* %flags_addr, align 4            ; <i32> [#uses=1]
  %11 = call i32 (...)* @fast_longjmp(i64* %9, i32 %10) nounwind ; <i32> [#uses=0]
  br label %bb1

bb1:                                              ; preds = %bb, %entry
  br label %return

return:                                           ; preds = %bb1
  ret void
}

declare i32 @fast_setjmp(...)

declare i32 @fast_longjmp(...)

define i32 @get_work_dim() nounwind {
entry:
  %retval = alloca i32                            ; <i32*> [#uses=2]
  %0 = alloca i32                                 ; <i32*> [#uses=2]
  %"alloca point" = bitcast i32 0 to i32          ; <i32> [#uses=0]
  %1 = call i8* @llvm.frameaddress(i32 0)         ; <i8*> [#uses=1]
  %2 = ptrtoint i8* %1 to i64                     ; <i64> [#uses=1]
  %3 = and i64 %2, -16384                         ; <i64> [#uses=1]
  %4 = inttoptr i64 %3 to %struct.vc_data*        ; <%struct.vc_data*> [#uses=1]
  %5 = getelementptr inbounds %struct.vc_data* %4, i32 0, i32 4 ; <%struct.work_struct**> [#uses=1]
  %6 = load %struct.work_struct** %5, align 8     ; <%struct.work_struct*> [#uses=1]
  %7 = getelementptr inbounds %struct.work_struct* %6, i32 0, i32 0 ; <i32*> [#uses=1]
  %8 = load i32* %7, align 8                      ; <i32> [#uses=1]
  store i32 %8, i32* %0, align 4
  %9 = load i32* %0, align 4                      ; <i32> [#uses=1]
  store i32 %9, i32* %retval, align 4
  br label %return

return:                                           ; preds = %entry
  %retval1 = load i32* %retval                    ; <i32> [#uses=1]
  ret i32 %retval1
}

declare i8* @llvm.frameaddress(i32) nounwind readnone

define i64 @get_local_size(i32 %d) nounwind {
entry:
  %d_addr = alloca i32                            ; <i32*> [#uses=2]
  %retval = alloca i64                            ; <i64*> [#uses=2]
  %0 = alloca i64                                 ; <i64*> [#uses=2]
  %"alloca point" = bitcast i32 0 to i32          ; <i32> [#uses=0]
  store i32 %d, i32* %d_addr
  %1 = call i8* @llvm.frameaddress(i32 0)         ; <i8*> [#uses=1]
  %2 = ptrtoint i8* %1 to i64                     ; <i64> [#uses=1]
  %3 = and i64 %2, -16384                         ; <i64> [#uses=1]
  %4 = inttoptr i64 %3 to %struct.vc_data*        ; <%struct.vc_data*> [#uses=1]
  %5 = getelementptr inbounds %struct.vc_data* %4, i32 0, i32 4 ; <%struct.work_struct**> [#uses=1]
  %6 = load %struct.work_struct** %5, align 8     ; <%struct.work_struct*> [#uses=1]
  %7 = load i32* %d_addr, align 4                 ; <i32> [#uses=1]
  %8 = getelementptr inbounds %struct.work_struct* %6, i32 0, i32 1 ; <[3 x i64]*> [#uses=1]
  %9 = zext i32 %7 to i64                         ; <i64> [#uses=1]
  %10 = getelementptr inbounds [3 x i64]* %8, i64 0, i64 %9 ; <i64*> [#uses=1]
  %11 = load i64* %10, align 8                    ; <i64> [#uses=1]
  store i64 %11, i64* %0, align 8
  %12 = load i64* %0, align 8                     ; <i64> [#uses=1]
  store i64 %12, i64* %retval, align 8
  br label %return

return:                                           ; preds = %entry
  %retval1 = load i64* %retval                    ; <i64> [#uses=1]
  ret i64 %retval1
}

define i64 @get_local_id(i32 %d) nounwind {
entry:
  %d_addr = alloca i32                            ; <i32*> [#uses=2]
  %retval = alloca i64                            ; <i64*> [#uses=2]
  %0 = alloca i64                                 ; <i64*> [#uses=2]
  %"alloca point" = bitcast i32 0 to i32          ; <i32> [#uses=0]
  store i32 %d, i32* %d_addr
  %1 = call i8* @llvm.frameaddress(i32 0)         ; <i8*> [#uses=1]
  %2 = ptrtoint i8* %1 to i64                     ; <i64> [#uses=1]
  %3 = and i64 %2, -16384                         ; <i64> [#uses=1]
  %4 = inttoptr i64 %3 to %struct.vc_data*        ; <%struct.vc_data*> [#uses=1]
  %5 = load i32* %d_addr, align 4                 ; <i32> [#uses=1]
  %6 = getelementptr inbounds %struct.vc_data* %4, i32 0, i32 5 ; <[3 x i64]*> [#uses=1]
  %7 = zext i32 %5 to i64                         ; <i64> [#uses=1]
  %8 = getelementptr inbounds [3 x i64]* %6, i64 0, i64 %7 ; <i64*> [#uses=1]
  %9 = load i64* %8, align 8                      ; <i64> [#uses=1]
  store i64 %9, i64* %0, align 8
  %10 = load i64* %0, align 8                     ; <i64> [#uses=1]
  store i64 %10, i64* %retval, align 8
  br label %return

return:                                           ; preds = %entry
  %retval1 = load i64* %retval                    ; <i64> [#uses=1]
  ret i64 %retval1
}

define i64 @get_num_groups(i32 %d) nounwind {
entry:
  %d_addr = alloca i32                            ; <i32*> [#uses=2]
  %retval = alloca i64                            ; <i64*> [#uses=2]
  %0 = alloca i64                                 ; <i64*> [#uses=2]
  %"alloca point" = bitcast i32 0 to i32          ; <i32> [#uses=0]
  store i32 %d, i32* %d_addr
  %1 = call i8* @llvm.frameaddress(i32 0)         ; <i8*> [#uses=1]
  %2 = ptrtoint i8* %1 to i64                     ; <i64> [#uses=1]
  %3 = and i64 %2, -16384                         ; <i64> [#uses=1]
  %4 = inttoptr i64 %3 to %struct.vc_data*        ; <%struct.vc_data*> [#uses=1]
  %5 = getelementptr inbounds %struct.vc_data* %4, i32 0, i32 4 ; <%struct.work_struct**> [#uses=1]
  %6 = load %struct.work_struct** %5, align 8     ; <%struct.work_struct*> [#uses=1]
  %7 = load i32* %d_addr, align 4                 ; <i32> [#uses=1]
  %8 = getelementptr inbounds %struct.work_struct* %6, i32 0, i32 2 ; <[3 x i64]*> [#uses=1]
  %9 = zext i32 %7 to i64                         ; <i64> [#uses=1]
  %10 = getelementptr inbounds [3 x i64]* %8, i64 0, i64 %9 ; <i64*> [#uses=1]
  %11 = load i64* %10, align 8                    ; <i64> [#uses=1]
  store i64 %11, i64* %0, align 8
  %12 = load i64* %0, align 8                     ; <i64> [#uses=1]
  store i64 %12, i64* %retval, align 8
  br label %return

return:                                           ; preds = %entry
  %retval1 = load i64* %retval                    ; <i64> [#uses=1]
  ret i64 %retval1
}

define i64 @get_global_size(i32 %d) nounwind {
entry:
  %d_addr = alloca i32                            ; <i32*> [#uses=2]
  %retval = alloca i64                            ; <i64*> [#uses=2]
  %0 = alloca i64                                 ; <i64*> [#uses=2]
  %"alloca point" = bitcast i32 0 to i32          ; <i32> [#uses=0]
  store i32 %d, i32* %d_addr
  %1 = call i8* @llvm.frameaddress(i32 0)         ; <i8*> [#uses=1]
  %2 = ptrtoint i8* %1 to i64                     ; <i64> [#uses=1]
  %3 = and i64 %2, -16384                         ; <i64> [#uses=1]
  %4 = inttoptr i64 %3 to %struct.vc_data*        ; <%struct.vc_data*> [#uses=1]
  %5 = getelementptr inbounds %struct.vc_data* %4, i32 0, i32 4 ; <%struct.work_struct**> [#uses=1]
  %6 = load %struct.work_struct** %5, align 8     ; <%struct.work_struct*> [#uses=1]
  %7 = load i32* %d_addr, align 4                 ; <i32> [#uses=1]
  %8 = getelementptr inbounds %struct.work_struct* %6, i32 0, i32 3 ; <[3 x i64]*> [#uses=1]
  %9 = zext i32 %7 to i64                         ; <i64> [#uses=1]
  %10 = getelementptr inbounds [3 x i64]* %8, i64 0, i64 %9 ; <i64*> [#uses=1]
  %11 = load i64* %10, align 8                    ; <i64> [#uses=1]
  store i64 %11, i64* %0, align 8
  %12 = load i64* %0, align 8                     ; <i64> [#uses=1]
  store i64 %12, i64* %retval, align 8
  br label %return

return:                                           ; preds = %entry
  %retval1 = load i64* %retval                    ; <i64> [#uses=1]
  ret i64 %retval1
}

define i64 @get_group_id(i32 %d) nounwind {
entry:
  %d_addr = alloca i32                            ; <i32*> [#uses=2]
  %retval = alloca i64                            ; <i64*> [#uses=2]
  %0 = alloca i64                                 ; <i64*> [#uses=2]
  %"alloca point" = bitcast i32 0 to i32          ; <i32> [#uses=0]
  store i32 %d, i32* %d_addr
  %1 = call i8* @llvm.frameaddress(i32 0)         ; <i8*> [#uses=1]
  %2 = ptrtoint i8* %1 to i64                     ; <i64> [#uses=1]
  %3 = and i64 %2, -16384                         ; <i64> [#uses=1]
  %4 = inttoptr i64 %3 to %struct.vc_data*        ; <%struct.vc_data*> [#uses=1]
  %5 = getelementptr inbounds %struct.vc_data* %4, i32 0, i32 4 ; <%struct.work_struct**> [#uses=1]
  %6 = load %struct.work_struct** %5, align 8     ; <%struct.work_struct*> [#uses=1]
  %7 = load i32* %d_addr, align 4                 ; <i32> [#uses=1]
  %8 = getelementptr inbounds %struct.work_struct* %6, i32 0, i32 4 ; <[3 x i64]*> [#uses=1]
  %9 = zext i32 %7 to i64                         ; <i64> [#uses=1]
  %10 = getelementptr inbounds [3 x i64]* %8, i64 0, i64 %9 ; <i64*> [#uses=1]
  %11 = load i64* %10, align 8                    ; <i64> [#uses=1]
  store i64 %11, i64* %0, align 8
  %12 = load i64* %0, align 8                     ; <i64> [#uses=1]
  store i64 %12, i64* %retval, align 8
  br label %return

return:                                           ; preds = %entry
  %retval1 = load i64* %retval                    ; <i64> [#uses=1]
  ret i64 %retval1
}

define i32 @get_global_id(i32 %d) nounwind {
entry:
  %d_addr = alloca i32                            ; <i32*> [#uses=3]
  %retval = alloca i32                            ; <i32*> [#uses=2]
  %data = alloca %struct.vc_data*                 ; <%struct.vc_data**> [#uses=3]
  %0 = alloca i32                                 ; <i32*> [#uses=2]
  %"alloca point" = bitcast i32 0 to i32          ; <i32> [#uses=0]
  store i32 %d, i32* %d_addr
  %1 = call i8* @llvm.frameaddress(i32 0)         ; <i8*> [#uses=1]
  %2 = ptrtoint i8* %1 to i64                     ; <i64> [#uses=1]
  %3 = and i64 %2, -16384                         ; <i64> [#uses=1]
  %4 = inttoptr i64 %3 to %struct.vc_data*        ; <%struct.vc_data*> [#uses=1]
  store %struct.vc_data* %4, %struct.vc_data** %data, align 8
  %5 = load i32* %d_addr, align 4                 ; <i32> [#uses=1]
  %6 = load %struct.vc_data** %data, align 8      ; <%struct.vc_data*> [#uses=1]
  %7 = getelementptr inbounds %struct.vc_data* %6, i32 0, i32 5 ; <[3 x i64]*> [#uses=1]
  %8 = zext i32 %5 to i64                         ; <i64> [#uses=1]
  %9 = getelementptr inbounds [3 x i64]* %7, i64 0, i64 %8 ; <i64*> [#uses=1]
  %10 = load i64* %9, align 8                     ; <i64> [#uses=1]
  %11 = trunc i64 %10 to i32                      ; <i32> [#uses=1]
  %12 = load %struct.vc_data** %data, align 8     ; <%struct.vc_data*> [#uses=1]
  %13 = getelementptr inbounds %struct.vc_data* %12, i32 0, i32 4 ; <%struct.work_struct**> [#uses=1]
  %14 = load %struct.work_struct** %13, align 8   ; <%struct.work_struct*> [#uses=1]
  %15 = load i32* %d_addr, align 4                ; <i32> [#uses=1]
  %16 = getelementptr inbounds %struct.work_struct* %14, i32 0, i32 5 ; <[3 x i64]*> [#uses=1]
  %17 = zext i32 %15 to i64                       ; <i64> [#uses=1]
  %18 = getelementptr inbounds [3 x i64]* %16, i64 0, i64 %17 ; <i64*> [#uses=1]
  %19 = load i64* %18, align 8                    ; <i64> [#uses=1]
  %20 = trunc i64 %19 to i32                      ; <i32> [#uses=1]
  %21 = add i32 %11, %20                          ; <i32> [#uses=1]
  store i32 %21, i32* %0, align 4
  %22 = load i32* %0, align 4                     ; <i32> [#uses=1]
  store i32 %22, i32* %retval, align 4
  br label %return

return:                                           ; preds = %entry
  %retval1 = load i32* %retval                    ; <i32> [#uses=1]
  ret i32 %retval1
}

define void @__prefetch_g4f32(i8* %p, i64 %n) nounwind {
entry:
  %p_addr = alloca i8*                            ; <i8**> [#uses=1]
  %n_addr = alloca i64                            ; <i64*> [#uses=1]
  %"alloca point" = bitcast i32 0 to i32          ; <i32> [#uses=0]
  store i8* %p, i8** %p_addr
  store i64 %n, i64* %n_addr
  br label %return

return:                                           ; preds = %entry
  ret void
}

define float @__rsqrt_f32(float %x) nounwind {
entry:
  %x_addr = alloca float                          ; <float*> [#uses=2]
  %retval = alloca float                          ; <float*> [#uses=2]
  %0 = alloca float                               ; <float*> [#uses=2]
  %"alloca point" = bitcast i32 0 to i32          ; <i32> [#uses=0]
  store float %x, float* %x_addr
  %1 = load float* %x_addr, align 4               ; <float> [#uses=1]
  %2 = fpext float %1 to double                   ; <double> [#uses=1]
  %3 = call double @llvm.sqrt.f64(double %2)      ; <double> [#uses=1]
  %4 = fdiv double 1.000000e+00, %3               ; <double> [#uses=1]
  %5 = fptrunc double %4 to float                 ; <float> [#uses=1]
  store float %5, float* %0, align 4
  %6 = load float* %0, align 4                    ; <float> [#uses=1]
  store float %6, float* %retval, align 4
  br label %return

return:                                           ; preds = %entry
  %retval1 = load float* %retval                  ; <float> [#uses=1]
  ret float %retval1
}

declare double @llvm.sqrt.f64(double) nounwind readonly

define float @__exp_f32(float %x) nounwind {
entry:
  %x_addr = alloca float                          ; <float*> [#uses=2]
  %retval = alloca float                          ; <float*> [#uses=2]
  %0 = alloca float                               ; <float*> [#uses=2]
  %"alloca point" = bitcast i32 0 to i32          ; <i32> [#uses=0]
  store float %x, float* %x_addr
  %1 = load float* %x_addr, align 4               ; <float> [#uses=1]
  %2 = call float @llvm.exp.f32(float %1)         ; <float> [#uses=1]
  store float %2, float* %0, align 4
  %3 = load float* %0, align 4                    ; <float> [#uses=1]
  store float %3, float* %retval, align 4
  br label %return

return:                                           ; preds = %entry
  %retval1 = load float* %retval                  ; <float> [#uses=1]
  ret float %retval1
}

declare float @llvm.exp.f32(float) nounwind readonly

define <4 x float> @__fabs_4f32(<4 x float> %x) nounwind {
entry:
  %x_addr = alloca <4 x float>                    ; <<4 x float>*> [#uses=4]
  %retval = alloca <4 x float>                    ; <<4 x float>*> [#uses=2]
  %m = alloca <4 x i32>                           ; <<4 x i32>*> [#uses=2]
  %0 = alloca <4 x float>                         ; <<4 x float>*> [#uses=2]
  %"alloca point" = bitcast i32 0 to i32          ; <i32> [#uses=0]
  store <4 x float> %x, <4 x float>* %x_addr
  store <4 x i32> <i32 2147483647, i32 2147483647, i32 2147483647, i32 2147483647>, <4 x i32>* %m, align 16
  call void asm "movaps $1,%xmm0\0A\09movaps $2,%xmm1\0A\09andps %xmm0,%xmm1\0A\09movaps %xmm1,$0\0A\09", "=*m,*m,*m,~{dirflag},~{fpsr},~{flags}"(<4 x float>* %x_addr, <4 x i32>* %m, <4 x float>* %x_addr) nounwind
  %1 = load <4 x float>* %x_addr, align 16        ; <<4 x float>> [#uses=1]
  store <4 x float> %1, <4 x float>* %0, align 16
  %2 = load <4 x float>* %0, align 16             ; <<4 x float>> [#uses=1]
  store <4 x float> %2, <4 x float>* %retval, align 16
  br label %return

return:                                           ; preds = %entry
  %retval1 = load <4 x float>* %retval            ; <<4 x float>> [#uses=1]
  ret <4 x float> %retval1
}
