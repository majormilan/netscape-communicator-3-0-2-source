/* -*- Mode: C; tab-width: 8 -*-
   scroller.c --- defines a subclass of XmScrolledWindow
   Copyright � 1994 Netscape Communications Corporation, all rights reserved.
   Created: Jamie Zawinski <jwz@netscape.com>, 23-Jul-94.
 */

#include <stdio.h>
#include <stdlib.h>
#include "mozilla.h"
#include "scroller.h"
#include "scrollerP.h"

static void scroller_resize (Widget);

/*
 * Motif 2.0 and later introduced a per-child constraint record
 * (XmScrolledWindowConstraintRec) on XmScrolledWindow that Motif 1.2
 * did not have.  A Constraint-class widget's constraint_size field is
 * *not* cumulative -- it is the amount of additional space that this
 * particular class contributes to the constraint record that Xt
 * allocates for each child.  Since Scroller adds no constraint fields
 * of its own, its constraint_size should just mirror whatever its
 * XmScrolledWindow superclass contributes at this Motif version:
 * nothing under 1.2, sizeof(XmScrolledWindowConstraintRec) under 2.0+.
 * Leaving this at 0 unconditionally (the old 1.2-only value) causes
 * Xt to under-allocate the constraint record for children created
 * inside a Scroller (e.g. the scrollbars in fe_MakeScrolledWindow),
 * which corrupts/segfaults inside libXm on current OpenMotif.
 */
#if XmVersion >= 2000
#define SCROLLER_CONSTRAINT_SIZE sizeof(XmScrolledWindowConstraintRec)
#else
#define SCROLLER_CONSTRAINT_SIZE 0
#endif

/*
 * Every Motif widget class -- and in particular every subclass of
 * XmManager (which XmScrolledWindow, and therefore Scroller, is) -- is
 * expected to carry a valid XmBaseClassExtRec in core_class.extension.
 * Older (Motif 1.1/1.2-era) toolkits tolerated this being NULL, but
 * current OpenMotif dereferences it unconditionally in places like the
 * keyboard-traversal/navigation code and the "fast subclass" machinery,
 * which crashes if core_class.extension is NULL.  Provide a minimal,
 * all-inherited extension record so Scroller behaves like any other
 * well-formed Manager subclass.
 */
static XmBaseClassExtRec _ScrollerCoreClassExtRec =
{
  /* next_extension          */  NULL,
  /* record_type             */  NULLQUARK, /* filled in by ScrollerClassInitialize(); XmQmotif isn't a compile-time constant */
  /* version                 */  XmBaseClassExtVersion,
  /* record_size             */  sizeof(XmBaseClassExtRec),
  /* initializePrehook       */  NULL,
  /* setValuesPrehook        */  NULL,
  /* initializePosthook      */  NULL,
  /* setValuesPosthook       */  NULL,
  /* secondaryObjectClass    */  NULL,
  /* secondaryObjectCreate   */  NULL,
  /* getSecResData           */  NULL,
  /* flags                   */  { 0 },
  /* getValuesPrehook        */  NULL,
  /* getValuesPosthook       */  NULL,
  /* classPartInitPrehook    */  NULL,
  /* classPartInitPosthook   */  NULL,
  /* ext_resources           */  NULL,
  /* compiled_ext_resources  */  NULL,
  /* num_ext_resources       */  0,
  /* use_sub_resources       */  FALSE,
  /* widgetNavigable         */  XmInheritWidgetNavigable,
  /* focusChange             */  XmInheritFocusChange,
  /* wrapperData             */  NULL,
};

/*
 * XmQmotif (the quark that marks a class-extension record as being the
 * Motif "base class" extension) is set up by the Xm/Xt runtime, not
 * available as a compile-time constant, so it has to be stored into our
 * static extension record once at class-initialize time rather than in
 * the initializer above.
 */
static void
ScrollerClassInitialize (void)
{
  _ScrollerCoreClassExtRec.record_type = XmQmotif;
}

ScrollerClassRec scrollerClassRec =
{
  {
/* core_class fields      */
    /* superclass         */    (WidgetClass) &xmScrolledWindowClassRec,
    /* class_name         */    "Scroller",
    /* widget_size        */    sizeof(ScrollerRec),
    /* class_initialize   */    (XtProc) ScrollerClassInitialize,
    /* class_partinit     */    NULL /* ClassPartInitialize */,
    /* class_inited       */	FALSE,
    /* initialize         */    NULL /* Initialize */,
    /* Init hook	  */    NULL,
    /* realize            */    XtInheritRealize /* Realize */,
    /* actions		  */	NULL /* ScrolledWActions */,
    /* num_actions	  */	0 /* XtNumber(ScrolledWActions) */,
    /* resources          */    NULL /* resources */,
    /* num_resources      */    0 /* XtNumber(resources) */,
    /* xrm_class          */    NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	TRUE,
    /* compress_enterleave*/	TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    scroller_resize,
    /* expose             */    XtInheritExpose /* (XtExposeProc) Redisplay */,
    /* set_values         */    NULL /* (XtSetValuesFunc )SetValues */,
    /* set values hook    */    NULL,
    /* set values almost  */    XtInheritSetValuesAlmost,
    /* get values hook    */    NULL,
    /* accept_focus       */    NULL,
    /* Version            */    XtVersion,
    /* PRIVATE cb list    */    NULL,
    /* tm_table		  */    XtInheritTranslations,
    /* query_geometry     */    XtInheritQueryGeometry /* QueryProc */,
    /* display_accelerator*/    NULL,
    /* extension          */    (XtPointer) &_ScrollerCoreClassExtRec,
  },
  {
/* composite_class fields */
    /* geometry_manager   */    XtInheritGeometryManager /*(XtGeometryHandler )GeometryManager*/,
    /* change_managed     */    XtInheritChangeManaged /*(XtWidgetProc) ChangeManaged*/,
    /* insert_child	  */	XtInheritInsertChild /*(XtArgsProc) InsertChild*/,	
    /* delete_child	  */	XtInheritDeleteChild,
    /* Extension          */    NULL,
  },{
/* Constraint class Init */
    NULL,
    0,
    SCROLLER_CONSTRAINT_SIZE,
    NULL,
    NULL,
    NULL,
    NULL
      
  },
/* Manager Class */
   {		
      XmInheritTranslations/*ScrolledWindowXlations*/,     /* translations        */    
      NULL /*get_resources*/,			/* get resources      	  */
      0 /*XtNumber(get_resources)*/,		/* num get_resources 	  */
      NULL,					/* get_cont_resources     */
      0,					/* num_get_cont_resources */
      XmInheritParentProcess,                   /* parent_process         */
      NULL,					/* extension           */    
   },

 {
/* Scrolled Window class - none */     
     /* mumble */               0
 },

 {
/* Scroller class - none */     
     /* mumble */               0
 }
};

WidgetClass scrollerClass = (WidgetClass)&scrollerClassRec;


static void scroller_resize (Widget widget)
{
  Scroller scroller = (Scroller) widget;

  /* Invoke the resize procedure of the superclass.
     Probably there's some nominally more portable way to do this
     (yeah right, like any of these slot names could possibly change
     and have any existing code still work.)
   */
  widget->core.widget_class->core_class.superclass->core_class.resize (widget);

  /* Now run our callback (yeah, I should use a real callback, so sue me.) */
  scroller->scroller.resize_hook (widget, scroller->scroller.resize_arg);
}
