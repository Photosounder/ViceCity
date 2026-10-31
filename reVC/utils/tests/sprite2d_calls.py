"""Adapt complete original 2D sprite calls while retaining native reference and temporary lifetimes."""
import re
from rect_migration import mask,closing,arguments
from sprite2d_schema import signatures


def choose(source,method,count):
    # Resolve overloads from complete original argument counts with instance and static names kept distinct
    rows=[r for r in signatures(source) if r[3]==method and len([] if r[4].strip() in ['', 'void'] else arguments(r[4]))==count]
    assert len(rows)==1,(method,count,[r[1] for r in rows]);return rows[0]


def adapter(row,raw,receiver=None):
    # Bind original native references and full-expression temporaries once before borrowing live scalar fields for C
    kind,name,_,old,original,*_=row;params=[] if original.strip() in ['', 'void'] else arguments(original);cpp=','.join((['Sprite2d *spriteSelf'] if receiver is not None else [])+params);values=['spriteSelf'] if receiver is not None else []
    for param in params:
        # Change only reference representation and retain every original scalar and SDK pointer conversion
        arg=re.search(r'(\w+)\s*$',param)[1]
        if 'const CRGBA &' in param:values.append('Sprite_ColourRef(&'+arg+'.r,&'+arg+'.g,&'+arg+'.b,&'+arg+'.a)')
        elif 'const CRect &' in param:values.append('&'+arg)
        else:values.append(arg)
    inputs=(receiver+(',' if raw.strip() else '') if receiver is not None else '')+raw
    return '([]('+cpp+'){\n    // Retain actual native reference objects through the complete synchronous C operation\n    '+('return ' if kind!='void' else '')+'Sprite2d_'+name+'('+','.join(values)+');\n})('+inputs+')'


def static_calls(text,owner):
    # Convert every explicit static member call without touching object data or unrelated sprite classes
    code=mask(text);changes=[]
    for match in re.finditer(r'\bCSprite2d::(\w+)\s*\(',code):
        # Preserve the exact original argument list and SDK vertex return type at existing native callers
        method=match[1];opening=code.index('(',match.start());end=closing(code,opening);raw=text[opening+1:end-1]
        if method=='GetVertices':new='((RwIm2DVertex*)Sprite2d_GetVertices())';assert not raw.strip()
        else:
            # Avoid introducing a reference adapter for signatures that already consist only of C-compatible scalar arguments
            count=0 if not raw.strip() else len(arguments(raw));row=choose(owner,method,count);new=adapter(row,raw) if '&' in row[4] else 'Sprite2d_'+row[1]+'('+raw+')'
        changes.append((match.start(),end,new))
    for first,last,new in reversed(changes):text=text[:first]+new+text[last:]
    return text


def receiver_start(code,end):
    # Recover the entire original receiver including namespace qualifiers member chains and balanced array indices
    first=end
    while first>0:
        ch=code[first-1]
        if ch in '])':
            # Keep complete index and parenthesized receiver expressions intact
            right=ch;left='[' if right==']' else '(';depth=1;first-=1
            while depth:
                first-=1
                if code[first]==right:depth+=1
                elif code[first]==left:depth-=1
        elif ch.isalnum() or ch in '_:.':first-=1
        elif ch=='>' and first>=2 and code[first-2]=='-':first-=2
        else:break
    return first


def member_calls(text,owner,names):
    # Convert only receivers diagnosed as actual 2D sprite records and preserve unrelated widget Draw methods
    code=mask(text);changes=[];rows=signatures(owner)
    for match in re.finditer(r'(\.|->)(Draw|Delete|SetTexture|SetAddressing|SetRenderState)\s*\(',code):
        # Identify the receiver outside array expressions so an index variable cannot masquerade as the object type
        first=receiver_start(code,match.start());receiver=text[first:match.start()];base=re.sub(r'\[(?:[^\[\]]|\[[^\[\]]*\])*\]','',code[first:match.start()]);identifiers=re.findall(r'\b\w+\b',base);key=identifiers[-1] if identifiers else ''
        if key not in names:continue
        if key=='m_sprites' and 'CPlaceableSprite::Draw' in text:continue
        method=match[2];opening=code.index('(',match.start());end=closing(code,opening);raw=text[opening+1:end-1];args=[] if not raw.strip() else arguments(raw);count=len(args)
        if method=='Draw' and count==5:
            # Distinguish the two five-argument instance overloads using their diagnosed original rectangle expressions
            target='DrawRectColours' if args[0].startswith(('Rect_','CRect_')) or args[0] in ['rect','r'] else 'DrawXYWH';row=next(r for r in rows if r[1]==target)
        else:row=choose(owner,method,count)
        pointer=receiver if match[1]=='->' else '&('+receiver+')';changes.append((first,end,adapter(row,raw,pointer)))
    for first,last,new in reversed(changes):text=text[:first]+new+text[last:]
    return text
