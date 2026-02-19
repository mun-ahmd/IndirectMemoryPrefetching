; ModuleID = 'bfs.cc'
source_filename = "bfs.cc"
target datalayout = "e-m:e-p:64:64-i64:64-i128:128-n64-S128"
target triple = "loongarch64-unknown-linux-gnu"

%union.pthread_mutex_t = type { %struct.__pthread_mutex_s }
%struct.__pthread_mutex_s = type { i32, i32, i32, i32, i32, i32, %struct.__pthread_internal_list }
%struct.__pthread_internal_list = type { ptr, ptr }
%struct.thread_arg_t = type { ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i32, i32, ptr, ptr }
%union.pthread_barrier_t = type { i64, [24 x i8] }
%struct.timespec = type { i64, i64 }

@lock = dso_local global %union.pthread_mutex_t zeroinitializer, align 8
@locks = dso_local local_unnamed_addr global ptr null, align 8
@local_min_buffer = dso_local global [1024 x i32] zeroinitializer, align 4
@global_min_buffer = dso_local global i32 0, align 4
@Total = dso_local local_unnamed_addr global i32 0, align 4
@terminate = dso_local local_unnamed_addr global i32 0, align 4
@P_global = dso_local local_unnamed_addr global i32 256, align 4
@edges = dso_local global ptr null, align 8
@exist = dso_local global ptr null, align 8
@temporary = dso_local global ptr null, align 8
@largest = dso_local local_unnamed_addr global i32 0, align 4
@thread_arg = dso_local global [1024 x %struct.thread_arg_t] zeroinitializer, align 8
@thread_handle = dso_local global [1024 x i64] zeroinitializer, align 8
@.str = private unnamed_addr constant [2 x i8] c"r\00", align 1
@.str.1 = private unnamed_addr constant [37 x i8] c"\0AGraph with Parameters: N:%d DEG:%d\0A\00", align 1
@stderr = external local_unnamed_addr global ptr, align 8
@.str.2 = private unnamed_addr constant [58 x i8] c"Degree of graph cannot be grater than number of Vertices\0A\00", align 1
@.str.3 = private unnamed_addr constant [29 x i8] c"Allocation of memory failed\0A\00", align 1
@.str.4 = private unnamed_addr constant [6 x i8] c"%d %d\00", align 1
@.str.5 = private unnamed_addr constant [52 x i8] c"Error: Read %d values, expected 2. Parsing failed.\0A\00", align 1
@.str.6 = private unnamed_addr constant [30 x i8] c"\0AFile Read, Largest Vertex:%d\00", align 1
@.str.7 = private unnamed_addr constant [17 x i8] c"\0AThreads Joined!\00", align 1
@.str.8 = private unnamed_addr constant [25 x i8] c"\0ATime Taken:\0A%lf seconds\00", align 1
@.str.9 = private unnamed_addr constant [11 x i8] c"myfile.txt\00", align 1
@.str.10 = private unnamed_addr constant [2 x i8] c"w\00", align 1
@.str.11 = private unnamed_addr constant [9 x i8] c"\0A %d %d \00", align 1

; Function Attrs: mustprogress nounwind
define dso_local noalias noundef ptr @_Z7do_workPv(ptr noundef %args) #0 {
entry:
  %tid1 = getelementptr inbounds %struct.thread_arg_t, ptr %args, i64 0, i32 6
  %0 = load volatile i32, ptr %tid1, align 8, !tbaa !4
  %P2 = getelementptr inbounds %struct.thread_arg_t, ptr %args, i64 0, i32 7
  %1 = load volatile i32, ptr %P2, align 4, !tbaa !10
  %Q3 = getelementptr inbounds %struct.thread_arg_t, ptr %args, i64 0, i32 2
  %2 = load volatile ptr, ptr %Q3, align 8, !tbaa !11
  %D4 = getelementptr inbounds %struct.thread_arg_t, ptr %args, i64 0, i32 3
  %3 = load volatile ptr, ptr %D4, align 8, !tbaa !12
  %W_index5 = getelementptr inbounds %struct.thread_arg_t, ptr %args, i64 0, i32 4
  %4 = load volatile ptr, ptr %W_index5, align 8, !tbaa !13
  %conv = sitofp i32 %1 to double
  %conv6 = sitofp i32 %0 to double
  %5 = load i32, ptr @largest, align 4, !tbaa !14
  %conv7 = sitofp i32 %5 to double
  %add = fadd double %conv7, 1.000000e+00
  %div = fdiv double %add, %conv
  %mul = fmul double %div, %conv6
  %add8 = fadd double %conv6, 1.000000e+00
  %mul10 = fmul double %add8, %div
  %conv11 = fptosi double %mul to i32
  %conv12 = fptosi double %mul10 to i32
  %barrier_total = getelementptr inbounds %struct.thread_arg_t, ptr %args, i64 0, i32 10
  %6 = load volatile ptr, ptr %barrier_total, align 8, !tbaa !15
  %call = tail call signext i32 @pthread_barrier_wait(ptr noundef %6) #12
  %7 = load i32, ptr @terminate, align 4, !tbaa !14
  %cmp133 = icmp eq i32 %7, 0
  br i1 %cmp133, label %for.cond.preheader.lr.ph, label %while.end

for.cond.preheader.lr.ph:                         ; preds = %entry
  %cmp13129 = icmp slt i32 %conv11, %conv12
  br i1 %cmp13129, label %for.cond.preheader.us.preheader, label %for.cond.preheader

for.cond.preheader.us.preheader:                  ; preds = %for.cond.preheader.lr.ph
  %8 = sext i32 %conv11 to i64
  %wide.trip.count = sext i32 %conv12 to i64
  br label %for.cond.preheader.us

for.cond.preheader.us:                            ; preds = %for.cond.preheader.us.preheader, %if.end80.us
  %iter.0134.us = phi i32 [ %inc81.us, %if.end80.us ], [ 0, %for.cond.preheader.us.preheader ]
  br label %for.body.us

for.end73.us:                                     ; preds = %for.inc71.us
  %9 = load i32, ptr @largest, align 4, !tbaa !14
  %idxprom74.us = sext i32 %9 to i64
  %arrayidx75.us = getelementptr inbounds i32, ptr %2, i64 %idxprom74.us
  %10 = load volatile i32, ptr %arrayidx75.us, align 4, !tbaa !14
  %cmp76.us = icmp ne i32 %10, 0
  %11 = load i32, ptr @Total, align 4
  %cmp78.not.us = icmp slt i32 %iter.0134.us, %11
  %or.cond.us = select i1 %cmp76.us, i1 %cmp78.not.us, i1 false
  br i1 %or.cond.us, label %if.end80.us, label %if.then79.us

if.then79.us:                                     ; preds = %for.end73.us
  store i32 1, ptr @terminate, align 4, !tbaa !14
  br label %if.end80.us

if.end80.us:                                      ; preds = %if.then79.us, %for.end73.us
  %inc81.us = add nuw nsw i32 %iter.0134.us, 1
  %12 = load volatile ptr, ptr %barrier_total, align 8, !tbaa !15
  %call83.us = tail call signext i32 @pthread_barrier_wait(ptr noundef %12) #12
  %13 = load i32, ptr @terminate, align 4, !tbaa !14
  %cmp.us = icmp eq i32 %13, 0
  br i1 %cmp.us, label %for.cond.preheader.us, label %while.end, !llvm.loop !16

for.body59.us:                                    ; preds = %for.body59.lr.ph.us, %for.inc71.us
  %indvars.iv168 = phi i64 [ %8, %for.body59.lr.ph.us ], [ %indvars.iv.next169, %for.inc71.us ]
  %arrayidx61.us = getelementptr inbounds i32, ptr %3, i64 %indvars.iv168
  %14 = load i32, ptr %arrayidx61.us, align 4, !tbaa !14
  %cmp62.us = icmp eq i32 %14, 1
  br i1 %cmp62.us, label %for.inc71.us, label %if.else.us

if.else.us:                                       ; preds = %for.body59.us
  %arrayidx67.us = getelementptr inbounds i32, ptr %33, i64 %indvars.iv168
  %15 = load i32, ptr %arrayidx67.us, align 4, !tbaa !14
  br label %for.inc71.us

for.inc71.us:                                     ; preds = %if.else.us, %for.body59.us
  %storemerge.us = phi i32 [ %15, %if.else.us ], [ 2, %for.body59.us ]
  store i32 %storemerge.us, ptr %arrayidx61.us, align 4, !tbaa !14
  %indvars.iv.next169 = add nsw i64 %indvars.iv168, 1
  %exitcond170.not = icmp eq i64 %indvars.iv.next169, %wide.trip.count
  br i1 %exitcond170.not, label %for.end73.us, label %for.body59.us, !llvm.loop !18

for.body.us:                                      ; preds = %for.cond.preheader.us, %for.inc52.us
  %indvars.iv166 = phi i64 [ %8, %for.cond.preheader.us ], [ %indvars.iv.next167, %for.inc52.us ]
  %16 = load ptr, ptr @exist, align 8, !tbaa !19
  %arrayidx.us = getelementptr inbounds i32, ptr %16, i64 %indvars.iv166
  %17 = load i32, ptr %arrayidx.us, align 4, !tbaa !14
  %cmp14.us = icmp eq i32 %17, 0
  br i1 %cmp14.us, label %for.inc52.us, label %if.end.us

if.end.us:                                        ; preds = %for.body.us
  %arrayidx16.us = getelementptr inbounds i32, ptr %3, i64 %indvars.iv166
  %18 = load i32, ptr %arrayidx16.us, align 4, !tbaa !14
  switch i32 %18, label %for.cond23.preheader.us [
    i32 0, label %for.inc52.us
    i32 2, label %for.inc52.us
  ]

for.body27.us:                                    ; preds = %for.body27.lr.ph.us, %if.end51.us
  %19 = phi ptr [ %30, %for.body27.lr.ph.us ], [ %27, %if.end51.us ]
  %indvars.iv164 = phi i64 [ 0, %for.body27.lr.ph.us ], [ %indvars.iv.next165, %if.end51.us ]
  %20 = load ptr, ptr %arrayidx29.us, align 8, !tbaa !19
  %arrayidx31.us = getelementptr inbounds i32, ptr %20, i64 %indvars.iv164
  %21 = load i32, ptr %arrayidx31.us, align 4, !tbaa !14
  %idxprom32.us = sext i32 %21 to i64
  %arrayidx33.us = getelementptr inbounds i32, ptr %2, i64 %idxprom32.us
  %22 = load volatile i32, ptr %arrayidx33.us, align 4, !tbaa !14
  %cmp34.us = icmp eq i32 %22, 1
  br i1 %cmp34.us, label %if.then35.us, label %if.end51.us

if.then35.us:                                     ; preds = %for.body27.us
  %23 = load ptr, ptr @locks, align 8, !tbaa !19
  %arrayidx37.us = getelementptr inbounds %union.pthread_mutex_t, ptr %23, i64 %idxprom32.us
  %call38.us = tail call signext i32 @pthread_mutex_lock(ptr noundef %arrayidx37.us) #12
  %24 = load volatile i32, ptr %arrayidx33.us, align 4, !tbaa !14
  %cmp41.us = icmp eq i32 %24, 1
  br i1 %cmp41.us, label %if.then42.us, label %if.end45.us

if.then42.us:                                     ; preds = %if.then35.us
  store volatile i32 0, ptr %arrayidx33.us, align 4, !tbaa !14
  br label %if.end45.us

if.end45.us:                                      ; preds = %if.then42.us, %if.then35.us
  %25 = load ptr, ptr @temporary, align 8, !tbaa !19
  %arrayidx47.us = getelementptr inbounds i32, ptr %25, i64 %idxprom32.us
  store i32 1, ptr %arrayidx47.us, align 4, !tbaa !14
  %26 = load ptr, ptr @locks, align 8, !tbaa !19
  %arrayidx49.us = getelementptr inbounds %union.pthread_mutex_t, ptr %26, i64 %idxprom32.us
  %call50.us = tail call signext i32 @pthread_mutex_unlock(ptr noundef %arrayidx49.us) #12
  %.pre = load ptr, ptr @edges, align 8, !tbaa !19
  br label %if.end51.us

if.end51.us:                                      ; preds = %if.end45.us, %for.body27.us
  %27 = phi ptr [ %.pre, %if.end45.us ], [ %19, %for.body27.us ]
  %indvars.iv.next165 = add nuw nsw i64 %indvars.iv164, 1
  %arrayidx25.us = getelementptr inbounds i32, ptr %27, i64 %indvars.iv166
  %28 = load i32, ptr %arrayidx25.us, align 4, !tbaa !14
  %29 = sext i32 %28 to i64
  %cmp26.us = icmp slt i64 %indvars.iv.next165, %29
  br i1 %cmp26.us, label %for.body27.us, label %for.inc52.us, !llvm.loop !20

for.inc52.us:                                     ; preds = %if.end51.us, %for.cond23.preheader.us, %if.end.us, %if.end.us, %for.body.us
  %indvars.iv.next167 = add nsw i64 %indvars.iv166, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next167 to i32
  %exitcond.not = icmp eq i32 %lftr.wideiv, %conv12
  br i1 %exitcond.not, label %for.body59.lr.ph.us, label %for.body.us, !llvm.loop !21

for.cond23.preheader.us:                          ; preds = %if.end.us
  %30 = load ptr, ptr @edges, align 8, !tbaa !19
  %arrayidx25126.us = getelementptr inbounds i32, ptr %30, i64 %indvars.iv166
  %31 = load i32, ptr %arrayidx25126.us, align 4, !tbaa !14
  %cmp26127.us = icmp sgt i32 %31, 0
  br i1 %cmp26127.us, label %for.body27.lr.ph.us, label %for.inc52.us

for.body27.lr.ph.us:                              ; preds = %for.cond23.preheader.us
  %arrayidx29.us = getelementptr inbounds ptr, ptr %4, i64 %indvars.iv166
  br label %for.body27.us

for.body59.lr.ph.us:                              ; preds = %for.inc52.us
  %32 = load volatile ptr, ptr %barrier_total, align 8, !tbaa !15
  %call56.us = tail call signext i32 @pthread_barrier_wait(ptr noundef %32) #12
  %33 = load ptr, ptr @temporary, align 8
  br label %for.body59.us

for.cond.preheader:                               ; preds = %for.cond.preheader.lr.ph, %if.end80
  %iter.0134 = phi i32 [ %inc81, %if.end80 ], [ 0, %for.cond.preheader.lr.ph ]
  %34 = load volatile ptr, ptr %barrier_total, align 8, !tbaa !15
  %call56 = tail call signext i32 @pthread_barrier_wait(ptr noundef %34) #12
  %35 = load i32, ptr @largest, align 4, !tbaa !14
  %idxprom74 = sext i32 %35 to i64
  %arrayidx75 = getelementptr inbounds i32, ptr %2, i64 %idxprom74
  %36 = load volatile i32, ptr %arrayidx75, align 4, !tbaa !14
  %cmp76 = icmp ne i32 %36, 0
  %37 = load i32, ptr @Total, align 4
  %cmp78.not = icmp slt i32 %iter.0134, %37
  %or.cond = select i1 %cmp76, i1 %cmp78.not, i1 false
  br i1 %or.cond, label %if.end80, label %if.then79

if.then79:                                        ; preds = %for.cond.preheader
  store i32 1, ptr @terminate, align 4, !tbaa !14
  br label %if.end80

if.end80:                                         ; preds = %for.cond.preheader, %if.then79
  %inc81 = add nuw nsw i32 %iter.0134, 1
  %38 = load volatile ptr, ptr %barrier_total, align 8, !tbaa !15
  %call83 = tail call signext i32 @pthread_barrier_wait(ptr noundef %38) #12
  %39 = load i32, ptr @terminate, align 4, !tbaa !14
  %cmp = icmp eq i32 %39, 0
  br i1 %cmp, label %for.cond.preheader, label %while.end, !llvm.loop !16

while.end:                                        ; preds = %if.end80, %if.end80.us, %entry
  %40 = load volatile ptr, ptr %barrier_total, align 8, !tbaa !15
  %call85 = tail call signext i32 @pthread_barrier_wait(ptr noundef %40) #12
  ret ptr null
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind
declare signext i32 @pthread_barrier_wait(ptr noundef) local_unnamed_addr #2

; Function Attrs: nounwind
declare signext i32 @pthread_mutex_lock(ptr noundef) local_unnamed_addr #2

; Function Attrs: nounwind
declare signext i32 @pthread_mutex_unlock(ptr noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress norecurse nounwind
define dso_local noundef signext i32 @main(i32 noundef signext %argc, ptr nocapture noundef readonly %argv) local_unnamed_addr #3 {
entry:
  %number0 = alloca i32, align 4
  %number1 = alloca i32, align 4
  %D = alloca ptr, align 8
  %Q = alloca ptr, align 8
  %d_count = alloca i32, align 4
  %barrier_total = alloca %union.pthread_barrier_t, align 8
  %barrier = alloca %union.pthread_barrier_t, align 8
  %requestStart = alloca %struct.timespec, align 8
  %requestEnd = alloca %struct.timespec, align 8
  %arrayidx = getelementptr inbounds ptr, ptr %argv, i64 1
  %0 = load ptr, ptr %arrayidx, align 8, !tbaa !19
  %call.i = tail call i64 @strtol(ptr nocapture noundef nonnull %0, ptr noundef null, i32 noundef signext 10) #12
  %conv.i = trunc i64 %call.i to i32
  %cmp = icmp eq i32 %conv.i, 1
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %arrayidx1 = getelementptr inbounds ptr, ptr %argv, i64 3
  %1 = load ptr, ptr %arrayidx1, align 8, !tbaa !19
  %call2 = tail call noalias ptr @fopen(ptr noundef %1, ptr noundef nonnull @.str)
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %spec.select365 = phi i32 [ 2097152, %if.then ], [ 0, %entry ]
  %spec.select = phi i32 [ 16, %if.then ], [ 0, %entry ]
  %file0.0 = phi ptr [ %call2, %if.then ], [ null, %entry ]
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %number0) #12
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %number1) #12
  %arrayidx6 = getelementptr inbounds ptr, ptr %argv, i64 2
  %2 = load ptr, ptr %arrayidx6, align 8, !tbaa !19
  %call.i366 = tail call i64 @strtol(ptr nocapture noundef nonnull %2, ptr noundef null, i32 noundef signext 10) #12
  %conv.i367 = trunc i64 %call.i366 to i32
  store i32 %conv.i367, ptr @P_global, align 4, !tbaa !14
  %cmp8 = icmp eq i32 %conv.i, 0
  br i1 %cmp8, label %if.then9, label %if.end15

if.then9:                                         ; preds = %if.end
  %arrayidx10 = getelementptr inbounds ptr, ptr %argv, i64 3
  %3 = load ptr, ptr %arrayidx10, align 8, !tbaa !19
  %call.i368 = tail call i64 @strtol(ptr nocapture noundef nonnull %3, ptr noundef null, i32 noundef signext 10) #12
  %conv.i369 = trunc i64 %call.i368 to i32
  %arrayidx12 = getelementptr inbounds ptr, ptr %argv, i64 4
  %4 = load ptr, ptr %arrayidx12, align 8, !tbaa !19
  %call.i370 = tail call i64 @strtol(ptr nocapture noundef nonnull %4, ptr noundef null, i32 noundef signext 10) #12
  %conv.i371 = trunc i64 %call.i370 to i32
  %call14 = tail call signext i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.1, i32 noundef signext %conv.i369, i32 noundef signext %conv.i371)
  br label %if.end15

if.end15:                                         ; preds = %if.then9, %if.end
  %DEG.1 = phi i32 [ %conv.i371, %if.then9 ], [ %spec.select, %if.end ]
  %N.1 = phi i32 [ %conv.i369, %if.then9 ], [ %spec.select365, %if.end ]
  %cmp16 = icmp sgt i32 %DEG.1, %N.1
  br i1 %cmp16, label %if.then17, label %if.end19

if.then17:                                        ; preds = %if.end15
  %5 = load ptr, ptr @stderr, align 8, !tbaa !19
  %6 = tail call i64 @fwrite(ptr nonnull @.str.2, i64 57, i64 1, ptr %5) #13
  tail call void @exit(i32 noundef signext 1) #14
  unreachable

if.end19:                                         ; preds = %if.end15
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %D) #12
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %Q) #12
  %conv = sext i32 %N.1 to i64
  %mul = shl nsw i64 %conv, 2
  %call20 = call signext i32 @posix_memalign(ptr noundef nonnull %D, i64 noundef 64, i64 noundef %mul) #12
  %tobool.not = icmp eq i32 %call20, 0
  br i1 %tobool.not, label %if.end23, label %if.then21

if.then21:                                        ; preds = %if.end19
  %7 = load ptr, ptr @stderr, align 8, !tbaa !19
  %8 = call i64 @fwrite(ptr nonnull @.str.3, i64 28, i64 1, ptr %7) #13
  call void @exit(i32 noundef signext 1) #14
  unreachable

if.end23:                                         ; preds = %if.end19
  %call26 = call signext i32 @posix_memalign(ptr noundef nonnull %Q, i64 noundef 64, i64 noundef %mul) #12
  %tobool27.not = icmp eq i32 %call26, 0
  br i1 %tobool27.not, label %if.end30, label %if.then28

if.then28:                                        ; preds = %if.end23
  %9 = load ptr, ptr @stderr, align 8, !tbaa !19
  %10 = call i64 @fwrite(ptr nonnull @.str.3, i64 28, i64 1, ptr %9) #13
  call void @exit(i32 noundef signext 1) #14
  unreachable

if.end30:                                         ; preds = %if.end23
  %call33 = call signext i32 @posix_memalign(ptr noundef nonnull @edges, i64 noundef 64, i64 noundef %mul) #12
  %tobool34.not = icmp eq i32 %call33, 0
  br i1 %tobool34.not, label %if.end37, label %if.then35

if.then35:                                        ; preds = %if.end30
  %11 = load ptr, ptr @stderr, align 8, !tbaa !19
  %12 = call i64 @fwrite(ptr nonnull @.str.3, i64 28, i64 1, ptr %11) #13
  call void @exit(i32 noundef signext 1) #14
  unreachable

if.end37:                                         ; preds = %if.end30
  %call40 = call signext i32 @posix_memalign(ptr noundef nonnull @exist, i64 noundef 64, i64 noundef %mul) #12
  %tobool41.not = icmp eq i32 %call40, 0
  br i1 %tobool41.not, label %if.end44, label %if.then42

if.then42:                                        ; preds = %if.end37
  %13 = load ptr, ptr @stderr, align 8, !tbaa !19
  %14 = call i64 @fwrite(ptr nonnull @.str.3, i64 28, i64 1, ptr %13) #13
  call void @exit(i32 noundef signext 1) #14
  unreachable

if.end44:                                         ; preds = %if.end37
  %call47 = call signext i32 @posix_memalign(ptr noundef nonnull @temporary, i64 noundef 64, i64 noundef %mul) #12
  %tobool48.not = icmp eq i32 %call47, 0
  br i1 %tobool48.not, label %if.end51, label %if.then49

if.then49:                                        ; preds = %if.end44
  %15 = load ptr, ptr @stderr, align 8, !tbaa !19
  %16 = call i64 @fwrite(ptr nonnull @.str.3, i64 28, i64 1, ptr %15) #13
  call void @exit(i32 noundef signext 1) #14
  unreachable

if.end51:                                         ; preds = %if.end44
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %d_count) #12
  store i32 %N.1, ptr %d_count, align 4, !tbaa !14
  call void @llvm.lifetime.start.p0(i64 32, ptr nonnull %barrier_total) #12
  call void @llvm.lifetime.start.p0(i64 32, ptr nonnull %barrier) #12
  %mul53 = shl nsw i64 %conv, 3
  %call54 = call noalias ptr @malloc(i64 noundef %mul53) #15
  %call57 = call noalias ptr @malloc(i64 noundef %mul53) #15
  %cmp58372 = icmp sgt i32 %N.1, 0
  br i1 %cmp58372, label %for.body.lr.ph, label %for.cond.cleanup79

for.body.lr.ph:                                   ; preds = %if.end51
  %conv60 = sext i32 %DEG.1 to i64
  %mul61 = shl nsw i64 %conv60, 2
  %wide.trip.count = zext nneg i32 %N.1 to i64
  br label %for.body

for.cond:                                         ; preds = %if.end66
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond77.preheader, label %for.body, !llvm.loop !22

for.cond77.preheader:                             ; preds = %for.cond
  br i1 %cmp58372, label %for.cond81.preheader.lr.ph, label %for.cond.cleanup79

for.cond81.preheader.lr.ph:                       ; preds = %for.cond77.preheader
  %cmp82374 = icmp sgt i32 %DEG.1, 0
  %17 = load ptr, ptr @edges, align 8, !tbaa !19
  %18 = load ptr, ptr @exist, align 8, !tbaa !19
  %19 = load ptr, ptr @temporary, align 8, !tbaa !19
  %wide.trip.count399 = zext nneg i32 %N.1 to i64
  br i1 %cmp82374, label %for.cond81.preheader.us.preheader, label %for.cond81.preheader

for.cond81.preheader.us.preheader:                ; preds = %for.cond81.preheader.lr.ph
  %wide.trip.count395 = zext nneg i32 %DEG.1 to i64
  br label %for.cond81.preheader.us

for.cond81.preheader.us:                          ; preds = %for.cond81.preheader.us.preheader, %for.cond81.for.cond.cleanup83_crit_edge.us
  %indvars.iv397 = phi i64 [ 0, %for.cond81.preheader.us.preheader ], [ %indvars.iv.next398, %for.cond81.for.cond.cleanup83_crit_edge.us ]
  %arrayidx86.us = getelementptr inbounds ptr, ptr %call57, i64 %indvars.iv397
  %20 = load ptr, ptr %arrayidx86.us, align 8, !tbaa !19
  br label %for.body84.us

for.body84.us:                                    ; preds = %for.cond81.preheader.us, %for.body84.us
  %indvars.iv393 = phi i64 [ 0, %for.cond81.preheader.us ], [ %indvars.iv.next394, %for.body84.us ]
  %arrayidx88.us = getelementptr inbounds i32, ptr %20, i64 %indvars.iv393
  store i32 100000000, ptr %arrayidx88.us, align 4, !tbaa !14
  %indvars.iv.next394 = add nuw nsw i64 %indvars.iv393, 1
  %exitcond396.not = icmp eq i64 %indvars.iv.next394, %wide.trip.count395
  br i1 %exitcond396.not, label %for.cond81.for.cond.cleanup83_crit_edge.us, label %for.body84.us, !llvm.loop !23

for.cond81.for.cond.cleanup83_crit_edge.us:       ; preds = %for.body84.us
  %arrayidx93.us = getelementptr inbounds i32, ptr %17, i64 %indvars.iv397
  store i32 0, ptr %arrayidx93.us, align 4, !tbaa !14
  %arrayidx95.us = getelementptr inbounds i32, ptr %18, i64 %indvars.iv397
  store i32 0, ptr %arrayidx95.us, align 4, !tbaa !14
  %arrayidx97.us = getelementptr inbounds i32, ptr %19, i64 %indvars.iv397
  store i32 0, ptr %arrayidx97.us, align 4, !tbaa !14
  %indvars.iv.next398 = add nuw nsw i64 %indvars.iv397, 1
  %exitcond400.not = icmp eq i64 %indvars.iv.next398, %wide.trip.count399
  br i1 %exitcond400.not, label %for.cond.cleanup79, label %for.cond81.preheader.us, !llvm.loop !24

for.body:                                         ; preds = %for.body.lr.ph, %for.cond
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.cond ]
  %arrayidx59 = getelementptr inbounds ptr, ptr %call54, i64 %indvars.iv
  %call62 = call signext i32 @posix_memalign(ptr noundef %arrayidx59, i64 noundef 64, i64 noundef %mul61) #12
  %tobool63.not = icmp eq i32 %call62, 0
  br i1 %tobool63.not, label %if.end66, label %if.then64

if.then64:                                        ; preds = %for.body
  %21 = load ptr, ptr @stderr, align 8, !tbaa !19
  %22 = call i64 @fwrite(ptr nonnull @.str.3, i64 28, i64 1, ptr %21) #13
  call void @exit(i32 noundef signext 1) #14
  unreachable

if.end66:                                         ; preds = %for.body
  %arrayidx68 = getelementptr inbounds ptr, ptr %call57, i64 %indvars.iv
  %call71 = call signext i32 @posix_memalign(ptr noundef %arrayidx68, i64 noundef 64, i64 noundef %mul61) #12
  %tobool72.not = icmp eq i32 %call71, 0
  br i1 %tobool72.not, label %for.cond, label %if.then73

if.then73:                                        ; preds = %if.end66
  %23 = load ptr, ptr @stderr, align 8, !tbaa !19
  %24 = call i64 @fwrite(ptr nonnull @.str.3, i64 28, i64 1, ptr %23) #13
  call void @exit(i32 noundef signext 1) #14
  unreachable

for.cond81.preheader:                             ; preds = %for.cond81.preheader.lr.ph, %for.cond81.preheader
  %indvars.iv389 = phi i64 [ %indvars.iv.next390, %for.cond81.preheader ], [ 0, %for.cond81.preheader.lr.ph ]
  %arrayidx93 = getelementptr inbounds i32, ptr %17, i64 %indvars.iv389
  store i32 0, ptr %arrayidx93, align 4, !tbaa !14
  %arrayidx95 = getelementptr inbounds i32, ptr %18, i64 %indvars.iv389
  store i32 0, ptr %arrayidx95, align 4, !tbaa !14
  %arrayidx97 = getelementptr inbounds i32, ptr %19, i64 %indvars.iv389
  store i32 0, ptr %arrayidx97, align 4, !tbaa !14
  %indvars.iv.next390 = add nuw nsw i64 %indvars.iv389, 1
  %exitcond392.not = icmp eq i64 %indvars.iv.next390, %wide.trip.count399
  br i1 %exitcond392.not, label %for.cond.cleanup79, label %for.cond81.preheader, !llvm.loop !24

for.cond.cleanup79:                               ; preds = %for.cond81.preheader, %for.cond81.for.cond.cleanup83_crit_edge.us, %if.end51, %for.cond77.preheader
  br i1 %cmp, label %for.cond105, label %if.end147

for.cond105:                                      ; preds = %for.cond.cleanup79, %for.cond105.backedge
  %lines_to_check.0 = phi i32 [ %lines_to_check.1, %for.cond105.backedge ], [ 0, %for.cond.cleanup79 ]
  %call143 = call signext i32 @getc(ptr noundef %file0.0)
  %conv144 = trunc i32 %call143 to i8
  switch i8 %conv144, label %if.end113 [
    i8 -1, label %for.end145
    i8 10, label %if.then111
  ]

if.then111:                                       ; preds = %for.cond105
  %inc112 = add nsw i32 %lines_to_check.0, 1
  br label %if.end113

if.end113:                                        ; preds = %for.cond105, %if.then111
  %lines_to_check.1 = phi i32 [ %inc112, %if.then111 ], [ %lines_to_check.0, %for.cond105 ]
  %cmp114 = icmp sgt i32 %lines_to_check.1, 3
  br i1 %cmp114, label %if.then115, label %for.cond105.backedge

for.cond105.backedge:                             ; preds = %if.end113, %if.end127
  br label %for.cond105, !llvm.loop !25

if.then115:                                       ; preds = %if.end113
  %call116 = call signext i32 (ptr, ptr, ...) @__isoc99_fscanf(ptr noundef %file0.0, ptr noundef nonnull @.str.4, ptr noundef nonnull %number0, ptr noundef nonnull %number1) #12
  switch i32 %call116, label %if.then119 [
    i32 -1, label %if.end121
    i32 2, label %if.end121
  ]

if.then119:                                       ; preds = %if.then115
  %call120 = call signext i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.5, i32 noundef signext %call116)
  call void @exit(i32 noundef signext 1) #14
  unreachable

if.end121:                                        ; preds = %if.then115, %if.then115
  %25 = load i32, ptr %number0, align 4, !tbaa !14
  %26 = load i32, ptr @largest, align 4, !tbaa !14
  %cmp122 = icmp sgt i32 %25, %26
  br i1 %cmp122, label %if.then123, label %if.end124

if.then123:                                       ; preds = %if.end121
  store i32 %25, ptr @largest, align 4, !tbaa !14
  br label %if.end124

if.end124:                                        ; preds = %if.then123, %if.end121
  %27 = phi i32 [ %25, %if.then123 ], [ %26, %if.end121 ]
  %28 = load i32, ptr %number1, align 4, !tbaa !14
  %cmp125 = icmp sgt i32 %28, %27
  br i1 %cmp125, label %if.then126, label %if.end127

if.then126:                                       ; preds = %if.end124
  store i32 %28, ptr @largest, align 4, !tbaa !14
  br label %if.end127

if.end127:                                        ; preds = %if.then126, %if.end124
  %29 = load ptr, ptr @edges, align 8, !tbaa !19
  %idxprom128 = sext i32 %25 to i64
  %arrayidx129 = getelementptr inbounds i32, ptr %29, i64 %idxprom128
  %30 = load i32, ptr %arrayidx129, align 4, !tbaa !14
  %arrayidx131 = getelementptr inbounds ptr, ptr %call57, i64 %idxprom128
  %31 = load ptr, ptr %arrayidx131, align 8, !tbaa !19
  %idxprom132 = sext i32 %30 to i64
  %arrayidx133 = getelementptr inbounds i32, ptr %31, i64 %idxprom132
  store i32 %28, ptr %arrayidx133, align 4, !tbaa !14
  %32 = load i32, ptr %number0, align 4, !tbaa !14
  %idxprom134 = sext i32 %32 to i64
  %arrayidx135 = getelementptr inbounds i32, ptr %29, i64 %idxprom134
  %33 = load i32, ptr %arrayidx135, align 4, !tbaa !14
  %inc136 = add nsw i32 %33, 1
  store i32 %inc136, ptr %arrayidx135, align 4, !tbaa !14
  %34 = load ptr, ptr @exist, align 8, !tbaa !19
  %35 = load i32, ptr %number0, align 4, !tbaa !14
  %idxprom137 = sext i32 %35 to i64
  %arrayidx138 = getelementptr inbounds i32, ptr %34, i64 %idxprom137
  store i32 1, ptr %arrayidx138, align 4, !tbaa !14
  %36 = load i32, ptr %number1, align 4, !tbaa !14
  %idxprom139 = sext i32 %36 to i64
  %arrayidx140 = getelementptr inbounds i32, ptr %34, i64 %idxprom139
  store i32 1, ptr %arrayidx140, align 4, !tbaa !14
  br label %for.cond105.backedge

for.end145:                                       ; preds = %for.cond105
  %37 = load i32, ptr @largest, align 4, !tbaa !14
  %call146 = call signext i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.6, i32 noundef signext %37)
  br label %if.end147

if.end147:                                        ; preds = %for.end145, %for.cond.cleanup79
  br i1 %cmp8, label %if.then149, label %if.end150

if.then149:                                       ; preds = %if.end147
  call void @_Z12init_weightsiiPPiS0_(i32 noundef signext %N.1, i32 noundef signext %DEG.1, ptr noundef %call54, ptr noundef %call57)
  %sub = add nsw i32 %N.1, -1
  store i32 %sub, ptr @largest, align 4, !tbaa !14
  br label %if.end150

if.end150:                                        ; preds = %if.then149, %if.end147
  %call151 = call signext i32 @pthread_barrier_init(ptr noundef nonnull %barrier_total, ptr noundef null, i32 noundef signext %conv.i367) #12
  %call152 = call signext i32 @pthread_barrier_init(ptr noundef nonnull %barrier, ptr noundef null, i32 noundef signext %conv.i367) #12
  %38 = load i32, ptr @largest, align 4, !tbaa !14
  %add = add nsw i32 %38, 16
  %conv153 = sext i32 %add to i64
  %mul154 = mul nsw i64 %conv153, 40
  %call155 = call noalias ptr @malloc(i64 noundef %mul154) #15
  store ptr %call155, ptr @locks, align 8, !tbaa !19
  %call156 = call signext i32 @pthread_mutex_init(ptr noundef nonnull @lock, ptr noundef null) #12
  %39 = load i32, ptr @largest, align 4, !tbaa !14
  %cmp160.not378 = icmp slt i32 %39, 0
  br i1 %cmp160.not378, label %for.cond.cleanup161, label %for.body162.preheader

for.body162.preheader:                            ; preds = %if.end150
  %.pre.pre420 = load ptr, ptr @exist, align 8, !tbaa !19
  br label %for.body162

for.cond.cleanup161:                              ; preds = %for.inc179, %if.end150
  %40 = load ptr, ptr %D, align 8, !tbaa !19
  %41 = load ptr, ptr %Q, align 8, !tbaa !19
  br i1 %cmp58372, label %for.body.preheader.i, label %_Z24initialize_single_sourcePiS_ii.exit

for.body.preheader.i:                             ; preds = %for.cond.cleanup161
  %wide.trip.count.i = zext nneg i32 %N.1 to i64
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %for.body.preheader.i
  %indvars.iv.i = phi i64 [ 0, %for.body.preheader.i ], [ %indvars.iv.next.i, %for.body.i ]
  %arrayidx.i = getelementptr inbounds i32, ptr %40, i64 %indvars.iv.i
  store i32 0, ptr %arrayidx.i, align 4, !tbaa !14
  %arrayidx2.i = getelementptr inbounds i32, ptr %41, i64 %indvars.iv.i
  store i32 1, ptr %arrayidx2.i, align 4, !tbaa !14
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %exitcond.not.i = icmp eq i64 %indvars.iv.next.i, %wide.trip.count.i
  br i1 %exitcond.not.i, label %_Z24initialize_single_sourcePiS_ii.exit, label %for.body.i, !llvm.loop !26

_Z24initialize_single_sourcePiS_ii.exit:          ; preds = %for.body.i, %for.cond.cleanup161
  store i32 1, ptr %40, align 4, !tbaa !14
  store i32 0, ptr %41, align 4, !tbaa !14
  %cmp185380 = icmp sgt i32 %conv.i367, 0
  br i1 %cmp185380, label %for.body187.preheader, label %for.cond.cleanup186.thread

for.cond.cleanup186.thread:                       ; preds = %_Z24initialize_single_sourcePiS_ii.exit
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %requestStart) #12
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %requestEnd) #12
  %call224423 = call signext i32 @clock_gettime(i32 noundef signext 0, ptr noundef nonnull %requestStart) #12
  br label %for.cond.cleanup228.thread

for.body187.preheader:                            ; preds = %_Z24initialize_single_sourcePiS_ii.exit
  %wide.trip.count405 = and i64 %call.i366, 4294967295
  br label %for.body187

for.body162:                                      ; preds = %for.body162.preheader, %for.inc179
  %.pre = phi ptr [ %.pre.pre420, %for.body162.preheader ], [ %.pre421, %for.inc179 ]
  %indvars.iv401 = phi i64 [ 0, %for.body162.preheader ], [ %indvars.iv.next402, %for.inc179 ]
  br i1 %cmp8, label %if.then164, label %if.end169

if.then164:                                       ; preds = %for.body162
  %arrayidx166 = getelementptr inbounds i32, ptr %.pre, i64 %indvars.iv401
  store i32 1, ptr %arrayidx166, align 4, !tbaa !14
  %42 = load ptr, ptr @edges, align 8, !tbaa !19
  %arrayidx168 = getelementptr inbounds i32, ptr %42, i64 %indvars.iv401
  store i32 %DEG.1, ptr %arrayidx168, align 4, !tbaa !14
  br label %if.end169

if.end169:                                        ; preds = %if.then164, %for.body162
  %arrayidx171 = getelementptr inbounds i32, ptr %.pre, i64 %indvars.iv401
  %43 = load i32, ptr %arrayidx171, align 4, !tbaa !14
  %cmp172 = icmp eq i32 %43, 1
  br i1 %cmp172, label %if.then173, label %for.inc179

if.then173:                                       ; preds = %if.end169
  %44 = load i32, ptr @Total, align 4, !tbaa !14
  %inc174 = add nsw i32 %44, 1
  store i32 %inc174, ptr @Total, align 4, !tbaa !14
  %45 = load ptr, ptr @locks, align 8, !tbaa !19
  %arrayidx176 = getelementptr inbounds %union.pthread_mutex_t, ptr %45, i64 %indvars.iv401
  %call177 = call signext i32 @pthread_mutex_init(ptr noundef %arrayidx176, ptr noundef null) #12
  %.pre.pre = load ptr, ptr @exist, align 8, !tbaa !19
  br label %for.inc179

for.inc179:                                       ; preds = %if.end169, %if.then173
  %.pre421 = phi ptr [ %.pre, %if.end169 ], [ %.pre.pre, %if.then173 ]
  %indvars.iv.next402 = add nuw nsw i64 %indvars.iv401, 1
  %46 = load i32, ptr @largest, align 4, !tbaa !14
  %47 = sext i32 %46 to i64
  %cmp160.not.not = icmp slt i64 %indvars.iv401, %47
  br i1 %cmp160.not.not, label %for.body162, label %for.cond.cleanup161, !llvm.loop !27

for.cond.cleanup186:                              ; preds = %for.body187
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %requestStart) #12
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %requestEnd) #12
  %call224 = call signext i32 @clock_gettime(i32 noundef signext 0, ptr noundef nonnull %requestStart) #12
  %cmp227382.not = icmp eq i32 %conv.i367, 1
  br i1 %cmp227382.not, label %for.cond.cleanup228.thread, label %for.body229.preheader

for.body229.preheader:                            ; preds = %for.cond.cleanup186
  %wide.trip.count409 = and i64 %call.i366, 4294967295
  br label %for.body229

for.body187:                                      ; preds = %for.body187.preheader, %for.body187
  %indvars.iv403 = phi i64 [ 0, %for.body187.preheader ], [ %indvars.iv.next404, %for.body187 ]
  %arrayidx189 = getelementptr inbounds [1024 x %struct.thread_arg_t], ptr @thread_arg, i64 0, i64 %indvars.iv403
  store ptr @local_min_buffer, ptr %arrayidx189, align 8, !tbaa !28
  %global_min = getelementptr inbounds [1024 x %struct.thread_arg_t], ptr @thread_arg, i64 0, i64 %indvars.iv403, i32 1
  store ptr @global_min_buffer, ptr %global_min, align 8, !tbaa !29
  %Q194 = getelementptr inbounds [1024 x %struct.thread_arg_t], ptr @thread_arg, i64 0, i64 %indvars.iv403, i32 2
  store ptr %41, ptr %Q194, align 8, !tbaa !11
  %D197 = getelementptr inbounds [1024 x %struct.thread_arg_t], ptr @thread_arg, i64 0, i64 %indvars.iv403, i32 3
  store ptr %40, ptr %D197, align 8, !tbaa !12
  %W_index200 = getelementptr inbounds [1024 x %struct.thread_arg_t], ptr @thread_arg, i64 0, i64 %indvars.iv403, i32 4
  store ptr %call57, ptr %W_index200, align 8, !tbaa !13
  %d_count203 = getelementptr inbounds [1024 x %struct.thread_arg_t], ptr @thread_arg, i64 0, i64 %indvars.iv403, i32 5
  store ptr %d_count, ptr %d_count203, align 8, !tbaa !30
  %tid = getelementptr inbounds [1024 x %struct.thread_arg_t], ptr @thread_arg, i64 0, i64 %indvars.iv403, i32 6
  %48 = trunc i64 %indvars.iv403 to i32
  store i32 %48, ptr %tid, align 8, !tbaa !4
  %P208 = getelementptr inbounds [1024 x %struct.thread_arg_t], ptr @thread_arg, i64 0, i64 %indvars.iv403, i32 7
  store i32 %conv.i367, ptr %P208, align 4, !tbaa !10
  %N211 = getelementptr inbounds [1024 x %struct.thread_arg_t], ptr @thread_arg, i64 0, i64 %indvars.iv403, i32 8
  store i32 %N.1, ptr %N211, align 8, !tbaa !31
  %DEG214 = getelementptr inbounds [1024 x %struct.thread_arg_t], ptr @thread_arg, i64 0, i64 %indvars.iv403, i32 9
  store i32 %DEG.1, ptr %DEG214, align 4, !tbaa !32
  %barrier_total217 = getelementptr inbounds [1024 x %struct.thread_arg_t], ptr @thread_arg, i64 0, i64 %indvars.iv403, i32 10
  store ptr %barrier_total, ptr %barrier_total217, align 8, !tbaa !15
  %barrier220 = getelementptr inbounds [1024 x %struct.thread_arg_t], ptr @thread_arg, i64 0, i64 %indvars.iv403, i32 11
  store ptr %barrier, ptr %barrier220, align 8, !tbaa !33
  %indvars.iv.next404 = add nuw nsw i64 %indvars.iv403, 1
  %exitcond406.not = icmp eq i64 %indvars.iv.next404, %wide.trip.count405
  br i1 %exitcond406.not, label %for.cond.cleanup186, label %for.body187, !llvm.loop !34

for.cond.cleanup228.thread:                       ; preds = %for.cond.cleanup186.thread, %for.cond.cleanup186
  %call236427 = call noundef ptr @_Z7do_workPv(ptr noundef nonnull @thread_arg)
  br label %for.cond.cleanup240

for.cond.cleanup228:                              ; preds = %for.body229
  %call236 = call noundef ptr @_Z7do_workPv(ptr noundef nonnull @thread_arg)
  br i1 %cmp227382.not, label %for.cond.cleanup240, label %for.body241.preheader

for.body241.preheader:                            ; preds = %for.cond.cleanup228
  %wide.trip.count413 = and i64 %call.i366, 4294967295
  br label %for.body241

for.body229:                                      ; preds = %for.body229.preheader, %for.body229
  %indvars.iv407 = phi i64 [ 1, %for.body229.preheader ], [ %indvars.iv.next408, %for.body229 ]
  %add.ptr = getelementptr inbounds i64, ptr @thread_handle, i64 %indvars.iv407
  %arrayidx231 = getelementptr inbounds [1024 x %struct.thread_arg_t], ptr @thread_arg, i64 0, i64 %indvars.iv407
  %call232 = call signext i32 @pthread_create(ptr noundef nonnull %add.ptr, ptr noundef null, ptr noundef nonnull @_Z7do_workPv, ptr noundef nonnull %arrayidx231) #12
  %indvars.iv.next408 = add nuw nsw i64 %indvars.iv407, 1
  %exitcond410.not = icmp eq i64 %indvars.iv.next408, %wide.trip.count409
  br i1 %exitcond410.not, label %for.cond.cleanup228, label %for.body229, !llvm.loop !35

for.cond.cleanup240:                              ; preds = %for.body241, %for.cond.cleanup228.thread, %for.cond.cleanup228
  %call248 = call signext i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.7)
  %call249 = call signext i32 @clock_gettime(i32 noundef signext 0, ptr noundef nonnull %requestEnd) #12
  %49 = load i64, ptr %requestEnd, align 8, !tbaa !36
  %50 = load i64, ptr %requestStart, align 8, !tbaa !36
  %sub251 = sub nsw i64 %49, %50
  %conv252 = sitofp i64 %sub251 to double
  %tv_nsec = getelementptr inbounds %struct.timespec, ptr %requestEnd, i64 0, i32 1
  %51 = load i64, ptr %tv_nsec, align 8, !tbaa !39
  %tv_nsec253 = getelementptr inbounds %struct.timespec, ptr %requestStart, i64 0, i32 1
  %52 = load i64, ptr %tv_nsec253, align 8, !tbaa !39
  %sub254 = sub nsw i64 %51, %52
  %conv255 = sitofp i64 %sub254 to double
  %div = fdiv double %conv255, 1.000000e+09
  %add256 = fadd double %div, %conv252
  %call257 = call signext i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.8, double noundef %add256)
  %call258 = call noalias ptr @fopen(ptr noundef nonnull @.str.9, ptr noundef nonnull @.str.10)
  %53 = load i32, ptr @largest, align 4, !tbaa !14
  %cmp261.not386 = icmp slt i32 %53, 0
  br i1 %cmp261.not386, label %for.cond.cleanup262, label %for.body263.preheader

for.body263.preheader:                            ; preds = %for.cond.cleanup240
  %.pre418 = load ptr, ptr @exist, align 8, !tbaa !19
  br label %for.body263

for.body241:                                      ; preds = %for.body241.preheader, %for.body241
  %indvars.iv411 = phi i64 [ 1, %for.body241.preheader ], [ %indvars.iv.next412, %for.body241 ]
  %arrayidx243 = getelementptr inbounds [1024 x i64], ptr @thread_handle, i64 0, i64 %indvars.iv411
  %54 = load i64, ptr %arrayidx243, align 8, !tbaa !40
  %call244 = call signext i32 @pthread_join(i64 noundef %54, ptr noundef null) #12
  %indvars.iv.next412 = add nuw nsw i64 %indvars.iv411, 1
  %exitcond414.not = icmp eq i64 %indvars.iv.next412, %wide.trip.count413
  br i1 %exitcond414.not, label %for.cond.cleanup240, label %for.body241, !llvm.loop !41

for.cond.cleanup262:                              ; preds = %for.inc272, %for.cond.cleanup240
  %putchar = call i32 @putchar(i32 10)
  %call276 = call signext i32 @fclose(ptr noundef %call258)
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %requestEnd) #12
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %requestStart) #12
  call void @llvm.lifetime.end.p0(i64 32, ptr nonnull %barrier) #12
  call void @llvm.lifetime.end.p0(i64 32, ptr nonnull %barrier_total) #12
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %d_count) #12
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %Q) #12
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %D) #12
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %number1) #12
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %number0) #12
  ret i32 0

for.body263:                                      ; preds = %for.body263.preheader, %for.inc272
  %55 = phi i32 [ %53, %for.body263.preheader ], [ %61, %for.inc272 ]
  %56 = phi ptr [ %.pre418, %for.body263.preheader ], [ %62, %for.inc272 ]
  %indvars.iv415 = phi i64 [ 0, %for.body263.preheader ], [ %indvars.iv.next416, %for.inc272 ]
  %arrayidx265 = getelementptr inbounds i32, ptr %56, i64 %indvars.iv415
  %57 = load i32, ptr %arrayidx265, align 4, !tbaa !14
  %cmp266 = icmp eq i32 %57, 1
  br i1 %cmp266, label %if.then267, label %for.inc272

if.then267:                                       ; preds = %for.body263
  %58 = load ptr, ptr %Q, align 8, !tbaa !19
  %arrayidx269 = getelementptr inbounds i32, ptr %58, i64 %indvars.iv415
  %59 = load i32, ptr %arrayidx269, align 4, !tbaa !14
  %60 = trunc i64 %indvars.iv415 to i32
  %call270 = call signext i32 (ptr, ptr, ...) @fprintf(ptr noundef %call258, ptr noundef nonnull @.str.11, i32 noundef signext %60, i32 noundef signext %59)
  %.pre417 = load ptr, ptr @exist, align 8, !tbaa !19
  %.pre419 = load i32, ptr @largest, align 4, !tbaa !14
  br label %for.inc272

for.inc272:                                       ; preds = %for.body263, %if.then267
  %61 = phi i32 [ %55, %for.body263 ], [ %.pre419, %if.then267 ]
  %62 = phi ptr [ %56, %for.body263 ], [ %.pre417, %if.then267 ]
  %indvars.iv.next416 = add nuw nsw i64 %indvars.iv415, 1
  %63 = sext i32 %61 to i64
  %cmp261.not.not = icmp slt i64 %indvars.iv415, %63
  br i1 %cmp261.not.not, label %for.body263, label %for.cond.cleanup262, !llvm.loop !42
}

; Function Attrs: nofree nounwind
declare noalias noundef ptr @fopen(ptr nocapture noundef readonly, ptr nocapture noundef readonly) local_unnamed_addr #4

; Function Attrs: nofree nounwind
declare noundef signext i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #4

; Function Attrs: nofree nounwind
declare noundef signext i32 @fprintf(ptr nocapture noundef, ptr nocapture noundef readonly, ...) local_unnamed_addr #4

; Function Attrs: noreturn nounwind
declare void @exit(i32 noundef signext) local_unnamed_addr #5

; Function Attrs: nofree nounwind
declare signext i32 @posix_memalign(ptr noundef, i64 noundef, i64 noundef) local_unnamed_addr #4

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #6

; Function Attrs: nofree nounwind
declare noundef signext i32 @getc(ptr nocapture noundef) local_unnamed_addr #4

declare signext i32 @__isoc99_fscanf(ptr noundef, ptr noundef, ...) local_unnamed_addr #7

; Function Attrs: mustprogress nounwind
define dso_local void @_Z12init_weightsiiPPiS0_(i32 noundef signext %N, i32 noundef signext %DEG, ptr nocapture noundef readonly %W, ptr nocapture noundef readonly %W_index) local_unnamed_addr #0 {
entry:
  %cmp159 = icmp sgt i32 %N, 0
  %cmp2157 = icmp sgt i32 %DEG, 0
  %or.cond = and i1 %cmp159, %cmp2157
  br i1 %or.cond, label %for.cond1.preheader.us.preheader, label %for.cond.cleanup74

for.cond1.preheader.us.preheader:                 ; preds = %entry
  %0 = zext nneg i32 %DEG to i64
  %1 = shl nuw nsw i64 %0, 2
  %wide.trip.count = zext nneg i32 %N to i64
  br label %for.cond1.preheader.us

for.cond1.preheader.us:                           ; preds = %for.cond1.preheader.us.preheader, %for.cond1.preheader.us
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader.us.preheader ], [ %indvars.iv.next, %for.cond1.preheader.us ]
  %arrayidx.us = getelementptr inbounds ptr, ptr %W_index, i64 %indvars.iv
  %2 = load ptr, ptr %arrayidx.us, align 8, !tbaa !19
  tail call void @llvm.memset.p0.i64(ptr align 4 %2, i8 -1, i64 %1, i1 false), !tbaa !14
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond11.preheader, label %for.cond1.preheader.us, !llvm.loop !43

for.cond11.preheader:                             ; preds = %for.cond1.preheader.us
  br i1 %cmp159, label %for.cond16.preheader.lr.ph, label %for.cond.cleanup74

for.cond16.preheader.lr.ph:                       ; preds = %for.cond11.preheader
  %cmp17161 = icmp sgt i32 %DEG, 0
  %sub = add nsw i32 %N, -1
  br i1 %cmp17161, label %for.cond16.preheader.us.preheader, label %for.cond.cleanup74

for.cond16.preheader.us.preheader:                ; preds = %for.cond16.preheader.lr.ph
  %wide.trip.count180 = zext nneg i32 %N to i64
  %wide.trip.count176 = zext nneg i32 %DEG to i64
  br label %for.cond16.preheader.us

for.cond16.preheader.us:                          ; preds = %for.cond16.preheader.us.preheader, %for.cond16.for.cond.cleanup18_crit_edge.us
  %indvars.iv178 = phi i64 [ 0, %for.cond16.preheader.us.preheader ], [ %indvars.iv.next179, %for.cond16.for.cond.cleanup18_crit_edge.us ]
  %arrayidx21.us = getelementptr inbounds ptr, ptr %W_index, i64 %indvars.iv178
  %3 = load ptr, ptr %arrayidx21.us, align 8, !tbaa !19
  br label %for.body19.us

for.body19.us:                                    ; preds = %for.cond16.preheader.us, %for.inc65.us
  %indvars.iv173 = phi i64 [ 0, %for.cond16.preheader.us ], [ %indvars.iv.next174, %for.inc65.us ]
  %last.0162.us = phi i32 [ 0, %for.cond16.preheader.us ], [ %last.2.us, %for.inc65.us ]
  %arrayidx23.us = getelementptr inbounds i32, ptr %3, i64 %indvars.iv173
  %4 = load i32, ptr %arrayidx23.us, align 4, !tbaa !14
  %cmp24.us = icmp eq i32 %4, -1
  br i1 %cmp24.us, label %if.then.us, label %if.end52.us

if.then.us:                                       ; preds = %for.body19.us
  %5 = add nuw nsw i64 %indvars.iv173, %indvars.iv178
  %6 = trunc i64 %5 to i32
  %cmp25.us = icmp slt i32 %last.0162.us, %6
  br i1 %cmp25.us, label %if.end52.us.sink.split, label %if.else.us

if.else.us:                                       ; preds = %if.then.us
  %cmp35.us = icmp slt i32 %last.0162.us, %sub
  br i1 %cmp35.us, label %if.then36.us, label %if.end52.us

if.then36.us:                                     ; preds = %if.else.us
  %add37.us = add nsw i32 %last.0162.us, 1
  br label %if.end52.us.sink.split

if.end52.us.sink.split:                           ; preds = %if.then.us, %if.then36.us
  %.sink = phi i32 [ %add37.us, %if.then36.us ], [ %6, %if.then.us ]
  store i32 %.sink, ptr %arrayidx23.us, align 4, !tbaa !14
  br label %if.end52.us

if.end52.us:                                      ; preds = %if.end52.us.sink.split, %if.else.us, %for.body19.us
  %7 = phi i32 [ -1, %if.else.us ], [ %4, %for.body19.us ], [ %.sink, %if.end52.us.sink.split ]
  %last.2.us = phi i32 [ %last.0162.us, %if.else.us ], [ %4, %for.body19.us ], [ %.sink, %if.end52.us.sink.split ]
  %cmp57.not.us = icmp slt i32 %7, %N
  br i1 %cmp57.not.us, label %for.inc65.us, label %if.then58.us

if.then58.us:                                     ; preds = %if.end52.us
  store i32 %sub, ptr %arrayidx23.us, align 4, !tbaa !14
  br label %for.inc65.us

for.inc65.us:                                     ; preds = %if.then58.us, %if.end52.us
  %indvars.iv.next174 = add nuw nsw i64 %indvars.iv173, 1
  %exitcond177.not = icmp eq i64 %indvars.iv.next174, %wide.trip.count176
  br i1 %exitcond177.not, label %for.cond16.for.cond.cleanup18_crit_edge.us, label %for.body19.us, !llvm.loop !44

for.cond16.for.cond.cleanup18_crit_edge.us:       ; preds = %for.inc65.us
  %indvars.iv.next179 = add nuw nsw i64 %indvars.iv178, 1
  %exitcond181.not = icmp eq i64 %indvars.iv.next179, %wide.trip.count180
  br i1 %exitcond181.not, label %for.cond72.preheader, label %for.cond16.preheader.us, !llvm.loop !45

for.cond72.preheader:                             ; preds = %for.cond16.for.cond.cleanup18_crit_edge.us
  br i1 %cmp159, label %for.cond77.preheader.us.preheader, label %for.cond.cleanup74

for.cond77.preheader.us.preheader:                ; preds = %for.cond72.preheader
  %wide.trip.count188 = zext nneg i32 %N to i64
  %wide.trip.count184 = zext nneg i32 %DEG to i64
  br label %for.cond77.preheader.us

for.cond77.preheader.us:                          ; preds = %for.cond77.preheader.us.preheader, %for.cond77.for.cond.cleanup79_crit_edge.us
  %indvars.iv186 = phi i64 [ 0, %for.cond77.preheader.us.preheader ], [ %indvars.iv.next187, %for.cond77.for.cond.cleanup79_crit_edge.us ]
  %arrayidx82.us = getelementptr inbounds ptr, ptr %W_index, i64 %indvars.iv186
  %arrayidx94.us = getelementptr inbounds ptr, ptr %W, i64 %indvars.iv186
  br label %for.body80.us

for.body80.us:                                    ; preds = %for.cond77.preheader.us, %for.body80.us
  %indvars.iv182 = phi i64 [ 0, %for.cond77.preheader.us ], [ %indvars.iv.next183, %for.body80.us ]
  %call.us = tail call double @drand48() #12
  %8 = load ptr, ptr %arrayidx82.us, align 8, !tbaa !19
  %arrayidx84.us = getelementptr inbounds i32, ptr %8, i64 %indvars.iv182
  %9 = load i32, ptr %arrayidx84.us, align 4, !tbaa !14
  %10 = zext i32 %9 to i64
  %cmp85.us = icmp eq i64 %indvars.iv186, %10
  %mul.us = fmul double %call.us, 1.000000e+02
  %conv.us = fptosi double %mul.us to i32
  %add92.us = add nsw i32 %conv.us, 1
  %.sink197 = select i1 %cmp85.us, i32 0, i32 %add92.us
  %11 = load ptr, ptr %arrayidx94.us, align 8, !tbaa !19
  %arrayidx90.us = getelementptr inbounds i32, ptr %11, i64 %indvars.iv182
  store i32 %.sink197, ptr %arrayidx90.us, align 4, !tbaa !14
  %indvars.iv.next183 = add nuw nsw i64 %indvars.iv182, 1
  %exitcond185.not = icmp eq i64 %indvars.iv.next183, %wide.trip.count184
  br i1 %exitcond185.not, label %for.cond77.for.cond.cleanup79_crit_edge.us, label %for.body80.us, !llvm.loop !46

for.cond77.for.cond.cleanup79_crit_edge.us:       ; preds = %for.body80.us
  %indvars.iv.next187 = add nuw nsw i64 %indvars.iv186, 1
  %exitcond189.not = icmp eq i64 %indvars.iv.next187, %wide.trip.count188
  br i1 %exitcond189.not, label %for.cond.cleanup74, label %for.cond77.preheader.us, !llvm.loop !47

for.cond.cleanup74:                               ; preds = %for.cond77.for.cond.cleanup79_crit_edge.us, %entry, %for.cond11.preheader, %for.cond16.preheader.lr.ph, %for.cond72.preheader
  ret void
}

; Function Attrs: nounwind
declare signext i32 @pthread_barrier_init(ptr noundef, ptr noundef, i32 noundef signext) local_unnamed_addr #2

; Function Attrs: nounwind
declare signext i32 @pthread_mutex_init(ptr noundef, ptr noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nofree norecurse nosync nounwind memory(argmem: write)
define dso_local noundef signext i32 @_Z24initialize_single_sourcePiS_ii(ptr nocapture noundef writeonly %D, ptr nocapture noundef writeonly %Q, i32 noundef signext %source, i32 noundef signext %N) local_unnamed_addr #8 {
entry:
  %cmp13 = icmp sgt i32 %N, 0
  br i1 %cmp13, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext nneg i32 %N to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  %idxprom3 = sext i32 %source to i64
  %arrayidx4 = getelementptr inbounds i32, ptr %D, i64 %idxprom3
  store i32 1, ptr %arrayidx4, align 4, !tbaa !14
  %arrayidx6 = getelementptr inbounds i32, ptr %Q, i64 %idxprom3
  store i32 0, ptr %arrayidx6, align 4, !tbaa !14
  ret i32 0

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %D, i64 %indvars.iv
  store i32 0, ptr %arrayidx, align 4, !tbaa !14
  %arrayidx2 = getelementptr inbounds i32, ptr %Q, i64 %indvars.iv
  store i32 1, ptr %arrayidx2, align 4, !tbaa !14
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !llvm.loop !26
}

; Function Attrs: nounwind
declare signext i32 @clock_gettime(i32 noundef signext, ptr noundef) local_unnamed_addr #2

; Function Attrs: nounwind
declare signext i32 @pthread_create(ptr noundef, ptr noundef, ptr noundef, ptr noundef) local_unnamed_addr #2

declare signext i32 @pthread_join(i64 noundef, ptr noundef) local_unnamed_addr #7

; Function Attrs: nofree nounwind
declare noundef signext i32 @fclose(ptr nocapture noundef) local_unnamed_addr #4

; Function Attrs: nounwind
declare double @drand48() local_unnamed_addr #2

; Function Attrs: mustprogress nofree nounwind willreturn
declare i64 @strtol(ptr noundef readonly, ptr nocapture noundef, i32 noundef signext) local_unnamed_addr #9

; Function Attrs: nofree nounwind
declare noundef i32 @putchar(i32 noundef signext) local_unnamed_addr #10

; Function Attrs: nofree nounwind
declare noundef i64 @fwrite(ptr nocapture noundef, i64 noundef, i64 noundef, ptr nocapture noundef) local_unnamed_addr #10

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #11

attributes #0 = { mustprogress nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="loongarch64" "target-features"="+64bit,+d,+f,+ual" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="loongarch64" "target-features"="+64bit,+d,+f,+ual" }
attributes #3 = { mustprogress norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="loongarch64" "target-features"="+64bit,+d,+f,+ual" }
attributes #4 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="loongarch64" "target-features"="+64bit,+d,+f,+ual" }
attributes #5 = { noreturn nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="loongarch64" "target-features"="+64bit,+d,+f,+ual" }
attributes #6 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="loongarch64" "target-features"="+64bit,+d,+f,+ual" }
attributes #7 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="loongarch64" "target-features"="+64bit,+d,+f,+ual" }
attributes #8 = { mustprogress nofree norecurse nosync nounwind memory(argmem: write) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="loongarch64" "target-features"="+64bit,+d,+f,+ual" }
attributes #9 = { mustprogress nofree nounwind willreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="loongarch64" "target-features"="+64bit,+d,+f,+ual" }
attributes #10 = { nofree nounwind }
attributes #11 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #12 = { nounwind }
attributes #13 = { cold }
attributes #14 = { noreturn nounwind }
attributes #15 = { nounwind allocsize(0) }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!4 = !{!5, !9, i64 48}
!5 = !{!"_ZTS12thread_arg_t", !6, i64 0, !6, i64 8, !6, i64 16, !6, i64 24, !6, i64 32, !6, i64 40, !9, i64 48, !9, i64 52, !9, i64 56, !9, i64 60, !6, i64 64, !6, i64 72}
!6 = !{!"any pointer", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}
!9 = !{!"int", !7, i64 0}
!10 = !{!5, !9, i64 52}
!11 = !{!5, !6, i64 16}
!12 = !{!5, !6, i64 24}
!13 = !{!5, !6, i64 32}
!14 = !{!9, !9, i64 0}
!15 = !{!5, !6, i64 64}
!16 = distinct !{!16, !17}
!17 = !{!"llvm.loop.mustprogress"}
!18 = distinct !{!18, !17}
!19 = !{!6, !6, i64 0}
!20 = distinct !{!20, !17}
!21 = distinct !{!21, !17}
!22 = distinct !{!22, !17}
!23 = distinct !{!23, !17}
!24 = distinct !{!24, !17}
!25 = distinct !{!25, !17}
!26 = distinct !{!26, !17}
!27 = distinct !{!27, !17}
!28 = !{!5, !6, i64 0}
!29 = !{!5, !6, i64 8}
!30 = !{!5, !6, i64 40}
!31 = !{!5, !9, i64 56}
!32 = !{!5, !9, i64 60}
!33 = !{!5, !6, i64 72}
!34 = distinct !{!34, !17}
!35 = distinct !{!35, !17}
!36 = !{!37, !38, i64 0}
!37 = !{!"_ZTS8timespec", !38, i64 0, !38, i64 8}
!38 = !{!"long", !7, i64 0}
!39 = !{!37, !38, i64 8}
!40 = !{!38, !38, i64 0}
!41 = distinct !{!41, !17}
!42 = distinct !{!42, !17}
!43 = distinct !{!43, !17}
!44 = distinct !{!44, !17}
!45 = distinct !{!45, !17}
!46 = distinct !{!46, !17}
!47 = distinct !{!47, !17}
