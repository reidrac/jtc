" Vim syntax file
" Language: Juan's Toy (jt)
" Maintainer: Juan J. Martinez
" Filenames: *.jt
" Latest Revision: 11 February 2014

if exists("b:current_syntax")
  finish
endif

syn case match

syn keyword jtDef def
syn keyword jtReturn return

hi def link jtDef Function
hi def link jtReturn Keyword

syn keyword jtConditional if else
syn keyword jtRepeat loop
syn keyword jtBuiltins typeof clone println
syn match jtDic "{}"

hi def link jtConditional Conditional
hi def link jtRepeat Repeat
hi def link jtBuiltins Keyword
hi def link jtDic Type

syn region jtComment start="#" end="$" contains=@Spell
syn region jtBlock start="{" end="}" transparent fold
syn region jtString start=+"+ skip=+\\\\\|\\"+ end=+"+ contains=@Spell

hi def link jtComment Comment
hi def link jtString String

syn match jtInteger "\<\d\+\>"
syn match jtFloat "\<\d\+\.\d*\>"

hi def link jtInteger Number
hi def link jtFloat Float

let b:current_syntax = "jt"

