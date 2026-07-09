#!/bin/bash
# Isolated-compile harness for the dBoxBox coloring search (decomp2).
# Usage: wf_harness.sh <candidate_src_relpath_from_SMS_ROOT> <id>
# Compiles the candidate to a unique .o (never touches the shared build) and
# objdiffs dBoxBox against the target. Emits ONE authoritative JSON line:
#   {"id","ok","dboxbox":<match%>,"diffrows","gaps","early","late",
#    "cats":{"SLOT","REG","SDA","OPCODE"},"err"}
# early = diff rows at addr < 0x3400 (R/Q dot-product spill region + TST block)
# late  = diff rows at addr >= 0x3400 (normal2 / center / sm contact-gen FP coloring)
# A WIN is dboxbox==100.0 with diffrows==0 and gaps==0.
set -u
SMS_ROOT=/home/yannick/Projects/decomp/smstrikers-decomp
cd "$SMS_ROOT" || exit 3
SRC="$1"; ID="$2"
WF=/tmp/wf; mkdir -p "$WF"
OUT="$WF/${ID}.o"; ERR="$WF/${ID}.err"
FUNC='dBoxBox__FPCfPCfPCfPCfPCfPCfPfPfPiiP12dContactGeomi'
TGT='build/G4QE01/obj/ode/collision_std.o'

FLAGS=(-nodefaults -proc gekko -align powerpc -enum int -fp hardware \
  -Cpp_exceptions off -O4,p -inline auto,deferred -maxerrors 1 -nosyspath \
  -fp_contract on -multibyte -pragma "cats off" -pragma "warn_notinlined off" \
  -RTTI off -char signed -use_lmw_stmw on -common off -cwd source -str reuse \
  -sym on -DdNODEBUG=ON -DdIDESINGLE -DdSINGLE=1 -DdTHREADING_INTF_DISABLED \
  -DHAVE_MALLOC_H=1 -i src/ode -i include \
  -i include/PowerPC_EABI_Support/MSL_C/MSL_Common/ -i include/ode -I- \
  -i include -i build/G4QE01/include \
  -i include/PowerPC_EABI_Support/MSL_C++/MSL_Common/ -lang=c++ \
  -inline auto -fp_contract on)

build/tools/wibo build/tools/sjiswrap.exe build/compilers/GC/2.0/mwcceppc.exe \
  "${FLAGS[@]}" -c "$SRC" -o "$OUT" >"$ERR" 2>&1
if [ $? -ne 0 ]; then
  MSG=$(tr '\n' ' ' < "$ERR" | sed 's/"/'"'"'/g' | cut -c1-300)
  echo "{\"id\":\"$ID\",\"ok\":false,\"dboxbox\":null,\"err\":\"COMPILE: $MSG\"}"
  exit 0
fi

objdiff-cli diff -1 "$TGT" -2 "$OUT" "$FUNC" -o "$WF/${ID}.json" --format json 2>/dev/null
python3 - "$WF/${ID}.json" "$ID" <<'PY'
import json,sys,re
p,ID=sys.argv[1],sys.argv[2]
try:
    d=json.load(open(p))
    def g(side):
        for s in side['symbols']:
            if s.get('name','').startswith('dBoxBox__'): return s
    ls=g(d['left']); rs=g(d['right'])
    mp=ls.get('match_percent')
    def rows(sym):
        out=[]
        for x in sym['instructions']:
            ins=x.get('instruction')
            out.append((ins['formatted'], int(ins['address'])) if ins else ('<GAP>',-1))
        return out
    L=rows(ls); R=rows(rs)
    gaps=sum(1 for f,_ in L if f=='<GAP>')+sum(1 for f,_ in R if f=='<GAP>')
    cats={'SLOT':0,'REG':0,'SDA':0,'OPCODE':0}
    early=late=diff=0
    if len(L)==len(R):
        for (fa,addr),(fb,_) in zip(L,R):
            if fa==fb: continue
            diff+=1
            if addr>=0 and addr<0x3400: early+=1
            elif addr>=0x3400: late+=1
            ma=fa.split()[0] if fa!='<GAP>' else ''; mb=fb.split()[0] if fb!='<GAP>' else ''
            ra=re.sub(r'0x[0-9a-f]+\(r1\)','S',fa); rb=re.sub(r'0x[0-9a-f]+\(r1\)','S',fb)
            sa=re.sub(r'@\d+@sda21','D',fa); sb=re.sub(r'@\d+@sda21','D',fb)
            if ma!=mb: cats['OPCODE']+=1
            elif ra==rb and 'S' in ra: cats['SLOT']+=1
            elif sa==sb and 'D' in sa: cats['SDA']+=1
            else: cats['REG']+=1
    else:
        diff=-1
    print(json.dumps({"id":ID,"ok":True,"dboxbox":round(mp,4) if mp is not None else None,
        "diffrows":diff,"gaps":gaps,"early":early,"late":late,"cats":cats,"err":""}))
except Exception as e:
    print(json.dumps({"id":ID,"ok":False,"dboxbox":None,"err":"PARSE:"+str(e)}))
PY
