
(in-package "USER")
(IN-PACKAGE "TK")
(AUTOLOAD 'MAKE-ONE-GRAPH '|gc-monitor|)
(AUTOLOAD 'PUSH-MULTIPLE-VALUES '|gc-monitor|)
(AUTOLOAD 'DRAW-STATUS '|gc-monitor|)
(AUTOLOAD 'DO-ALLOCATION '|gc-monitor|)
(AUTOLOAD 'SET-LABEL-BACKGROUND '|gc-monitor|)
(AUTOLOAD 'MKGCMONITOR '|gc-monitor|)
(IN-PACKAGE "TK")
(AUTOLOAD 'MKBASIC '|mkBasic|)
(IN-PACKAGE "TK")
(AUTOLOAD 'MKCANVTEXT '|mkCanvText|)
(AUTOLOAD 'MKTEXTCONFIG '|mkCanvText|)
(AUTOLOAD 'TEXTENTER '|mkCanvText|)
(AUTOLOAD 'TEXTB1PRESS '|mkCanvText|)
(AUTOLOAD 'TEXTB1MOVE '|mkCanvText|)
(AUTOLOAD 'TEXTBS '|mkCanvText|)
(IN-PACKAGE "TK")
(AUTOLOAD 'MKENTRY '|mkEntry|)
(IN-PACKAGE "TK")
(AUTOLOAD 'MKENTRY2 '|mkEntry2|)
(IN-PACKAGE "TK")
(AUTOLOAD 'MKFORM '|mkForm|)
(AUTOLOAD 'TAB '|mkForm|)
(IN-PACKAGE "TK")
(AUTOLOAD 'MKHSCALE '|mkHScale|)
(AUTOLOAD 'SETWIDTH '|mkHScale|)
(IN-PACKAGE "TK")
(AUTOLOAD 'MKITEMS '|mkItems|)
(AUTOLOAD 'ITEMENTER '|mkItems|)
(AUTOLOAD 'ITEMLEAVE '|mkItems|)
(AUTOLOAD 'ITEMMARK '|mkItems|)
(AUTOLOAD 'ITEMSTROKE '|mkItems|)
(AUTOLOAD 'ITEMSUNDERAREA '|mkItems|)
(AUTOLOAD 'ITEMSTARTDRAG '|mkItems|)
(AUTOLOAD 'ITEMDRAG '|mkItems|)
(AUTOLOAD 'ITEMDRAG '|mkItems|)
(AUTOLOAD 'BUTPRESS '|mkItems|)
(AUTOLOAD 'EMACS-MOVE '|mkItems|)
(IN-PACKAGE "TK")
(AUTOLOAD 'MKLABEL '|mkLabel|)
(IN-PACKAGE "TK")
(AUTOLOAD 'MKLISTBOX '|mkListbox|)
(IN-PACKAGE "TK")
(AUTOLOAD 'MKPLOT '|mkPlot|)
(AUTOLOAD 'PLOTDOWN '|mkPlot|)
(AUTOLOAD 'PLOTMOVE '|mkPlot|)
(IN-PACKAGE "TK")
(AUTOLOAD 'MKRADIO '|mkRadio|)
(IN-PACKAGE "TK")
(AUTOLOAD 'MKRULER '|mkRuler|)
(AUTOLOAD 'RULERMKTAB '|mkRuler|)
(AUTOLOAD 'RULERNEWTAB '|mkRuler|)
(AUTOLOAD 'RULERMOVETAB '|mkRuler|)
(AUTOLOAD 'DEMO_SELECTTAB '|mkRuler|)
(AUTOLOAD 'RULERRELEASETAB '|mkRuler|)
(IN-PACKAGE "TK")
(AUTOLOAD 'MKTEXTSEARCH '|mkSearch|)
(AUTOLOAD 'TEXTLOADFILE '|mkSearch|)
(AUTOLOAD 'TEXTSEARCH '|mkSearch|)
(IN-PACKAGE "TK")
(AUTOLOAD 'MKSTYLES '|mkStyles|)
(AUTOLOAD 'INSERTWITHTAGS '|mkStyles|)
(IN-PACKAGE "TK")
(AUTOLOAD 'MKTEXTBIND '|mkTextBind|)
(AUTOLOAD 'INSERTWITHTAGS '|mkTextBind|)
(IN-PACKAGE "TK")
(AUTOLOAD 'MKVSCALE '|mkVScale|)
(AUTOLOAD 'SETHEIGHT '|mkVScale|)
(IN-PACKAGE "TK")
(AUTOLOAD 'MKDIALOG '|mkdialog|)
(IN-PACKAGE "TK")
(AUTOLOAD 'NQTHM-STACK '|nqthm-stack|)
(IN-PACKAGE "USER")
(SYSTEM::AUTOLOAD 'TK::SHOW-FRAMES '|nqthm-stack|)
(SYSTEM::AUTOLOAD 'DISPLAY-REWRITE-PATH-TOKEN '|nqthm-stack|)
(IN-PACKAGE "TK")
(AUTOLOAD 'SHOWVARS '|showVars|)
(IN-PACKAGE "TK")
(AUTOLOAD 'DPOS '|widget|)
(in-package "USER")

(SETQ *LOAD-PATH*
      (APPEND '("/fix/f/debian/mm/gcl/gcl-2.5.0/gcl-tk/demos/")
              *LOAD-PATH*))