@begin(format)
@tabclear()
@tabset(.5in,3in,4in,5in)
@>/* @!@F9{Rog-O-Matic XIII,  Happy Hackers, Inc. Carnegie-Mellon University}
@/********************************************************************
@/*								    
@/*                 @F9{Copyright (c) 1982, 1983 by                      }
@/*								    
@/*  @F9{Andrew Appel, Leonard Hamey, Guy Jacobson, and Michael Mauldin  }
@/*								    
@/********************************************************************/

@//* @!@F9{strike.c: version XIII, 11/01/83 }*/

@//* 
 @!* @F9{strike.c: Leave a quit message for the umpire command.}
 */

@F6{#} @F6{include} <@F6{stdio}.@F6{h}>
@F6{#} @F6{define} @F6{QUITFILE} "/@F8{usr}/@F8{mlm}/.@F8{quitumpire}"

@F6{main} (@F6{argc}, @F6{argv})
@F7{int} @F6{argc};
@F7{char} *@F6{argv}[];
{ 
  @F7{if} (@F6{argc} <= @F6{1})
  { @F7{if} (@F6{access} (@F6{QUITFILE}, @F6{4}))
    { @F7{if} (@F6{creat} (@F6{QUITFILE}, @F6{0600}) == @F6{EOF})
        @F6{fprintf} (@F6{stderr}, "@F8{Cannot} @F8{create} %@F8{s}.\@F8{n}", @F6{QUITFILE});
      @F7{else}
        @F6{fprintf} (@F6{stderr}, "@F8{Quit} @F8{message} @F8{left} @F8{for} @F8{umpire}.\@F8{n}");
    }
    @F7{else}
      @F6{fprintf} (@F6{stderr}, "@F8{Umpire} @F8{already} @F8{has} @F8{a} @F8{quit} @F8{message}.\@F8{n}", @F6{QUITFILE});
  }
  @F7{else}
  { @F7{if} (@F6{access} (@F6{QUITFILE}, @F6{4}))
      @F6{fprintf} (@F6{stderr}, "@F8{Umpire} @F8{has} @F8{no} @F8{quit} @F8{message}!\@F8{n}");
    @F7{else}
    { @F6{unlink} (@F6{QUITFILE});
      @F6{fprintf} (@F6{stderr}, "@F8{Quit} @F8{message} @F8{removed}!\@F8{n}");
    }
  }
}
@end(format)
