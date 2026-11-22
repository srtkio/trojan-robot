#pragma once
// The TROJAN remote: one self-contained page served from flash. Phone-first, keyboard + gamepad on laptop.
static const char REMOTE_HTML[] PROGMEM = R"TROJANHTML(<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no,viewport-fit=cover">
<meta name="theme-color" content="#0a0d13"><title>TROJAN</title>
<style>
:root{--bg:#0a0d13;--p1:#121823;--p2:#1a2230;--line:#263146;--tx:#e8edf5;--mut:#8593a8;--acc:#ff8a3d;--acc2:#35d6ff;--ok:#3ddc84;--bad:#ff4d5e;--r:14px}
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
html,body{margin:0;height:100%;background:var(--bg);color:var(--tx);font:15px/1.4 system-ui,-apple-system,Segoe UI,Roboto,sans-serif;overscroll-behavior:none}
header{position:fixed;top:0;left:0;right:0;z-index:5;display:flex;align-items:center;gap:10px;padding:calc(8px + env(safe-area-inset-top)) 14px 8px;background:rgba(10,13,19,.92);backdrop-filter:blur(8px);border-bottom:1px solid var(--line)}
.logo{font-weight:800;letter-spacing:.18em;font-size:17px}.logo b{color:var(--acc)}
.dot{width:10px;height:10px;border-radius:50%;background:var(--bad);box-shadow:0 0 8px currentColor;color:var(--bad)}.dot.on{background:var(--ok);color:var(--ok)}
.chip{font-size:12px;color:var(--mut);padding:3px 9px;border:1px solid var(--line);border-radius:99px;white-space:nowrap}
.grow{flex:1}
#estop{background:var(--bad);color:#fff;border:0;border-radius:10px;font-weight:800;letter-spacing:.08em;padding:9px 14px;font-size:14px}
main{padding:calc(62px + env(safe-area-inset-top)) 12px calc(76px + env(safe-area-inset-bottom));max-width:1100px;margin:0 auto}
nav{position:fixed;bottom:0;left:0;right:0;z-index:5;display:flex;background:rgba(10,13,19,.96);border-top:1px solid var(--line);padding-bottom:env(safe-area-inset-bottom)}
nav button{flex:1;background:none;border:0;color:var(--mut);padding:10px 2px 9px;font-size:11px;font-weight:600;letter-spacing:.04em}
nav button svg{display:block;margin:0 auto 3px;width:22px;height:22px;stroke:currentColor;fill:none;stroke-width:1.8;stroke-linecap:round;stroke-linejoin:round}
nav button.on{color:var(--acc)}
.tab{display:none}.tab.on{display:block}
.card{background:var(--p1);border:1px solid var(--line);border-radius:var(--r);padding:12px;margin-bottom:12px}
.card h3{margin:0 0 10px;font-size:12px;letter-spacing:.14em;text-transform:uppercase;color:var(--mut)}
.row{display:flex;gap:8px;flex-wrap:wrap;align-items:center}
.grid{display:grid;gap:8px;grid-template-columns:repeat(auto-fill,minmax(92px,1fr))}
button.b,.b{background:var(--p2);color:var(--tx);border:1px solid var(--line);border-radius:11px;padding:11px 8px;font-size:14px;font-weight:600;min-height:44px}
button.b:active{transform:scale(.97);background:#222d40}
.b.acc{background:var(--acc);border-color:var(--acc);color:#1a0f05}.b.on{border-color:var(--acc2);color:var(--acc2)}
.b.danger{border-color:var(--bad);color:var(--bad)}
label{display:block;font-size:12px;color:var(--mut);margin:8px 0 2px}label span{float:right;color:var(--tx);font-variant-numeric:tabular-nums}
input[type=range]{width:100%;accent-color:var(--acc);height:28px;margin:0}
input[type=text],input[type=number],select,textarea{width:100%;background:var(--p2);border:1px solid var(--line);color:var(--tx);border-radius:10px;padding:10px;font-size:15px}
.cols{display:grid;gap:12px}
@media(min-width:760px){.cols.c2{grid-template-columns:1fr 1fr}.cols.c3{grid-template-columns:1fr 1fr 1fr}}
/* drive */
#driveGrid{display:grid;gap:12px;grid-template-columns:1fr 1fr;grid-template-areas:"view view" "sl sr" "ctl ctl"}
#view{grid-area:view}#sl{grid-area:sl}#sr{grid-area:sr}#ctl{grid-area:ctl}
@media(orientation:landscape) and (max-height:520px){main{padding-left:calc(12px + env(safe-area-inset-left));padding-right:calc(12px + env(safe-area-inset-right))}#driveGrid{grid-template-columns:230px 1fr 230px;grid-template-areas:"sl view sr" "sl ctl sr"}}
.stickWrap{display:flex;flex-direction:column;align-items:center;gap:6px;touch-action:none}
.stick{position:relative;width:170px;height:170px;border-radius:50%;background:radial-gradient(circle,#1b2535,#0f1520);border:2px solid var(--line);touch-action:none}
.stick:before{content:"";position:absolute;inset:50% 8% auto 8%;height:1px;background:var(--line)}.stick:after{content:"";position:absolute;left:50%;top:8%;bottom:8%;width:1px;background:var(--line)}
.knob{position:absolute;left:50%;top:50%;width:68px;height:68px;margin:-34px 0 0 -34px;border-radius:50%;background:radial-gradient(circle at 35% 30%,#ffb27a,var(--acc));box-shadow:0 4px 18px rgba(255,138,61,.45);z-index:2}
.stick.r .knob{background:radial-gradient(circle at 35% 30%,#9cecff,var(--acc2));box-shadow:0 4px 18px rgba(53,214,255,.4)}
.cap{font-size:11px;color:var(--mut);letter-spacing:.12em;text-transform:uppercase}
canvas#sim{width:100%;height:230px;background:#0c1119;border-radius:12px;display:block}
.seg{display:flex;border:1px solid var(--line);border-radius:10px;overflow:hidden}.seg button{flex:1;background:var(--p2);border:0;color:var(--mut);padding:10px;font-weight:700}.seg button.on{background:var(--acc2);color:#04222a}
.legg{display:grid;grid-template-columns:repeat(4,1fr);gap:8px}.legg h4{margin:0;font-size:12px;color:var(--acc);text-align:center}
.item{display:flex;align-items:center;gap:8px;padding:9px 10px;border:1px solid var(--line);border-radius:11px;background:var(--p2);margin-bottom:8px}
.item .n{flex:1;font-weight:700}.item .t{color:var(--mut);font-size:12px}
.item button{background:var(--p1);border:1px solid var(--line);color:var(--tx);border-radius:9px;padding:7px 11px;font-size:13px}
#fcv{width:100%;max-width:512px;aspect-ratio:2/1;background:#000;border:2px solid var(--line);border-radius:6px;image-rendering:pixelated;touch-action:none;display:block}
table{width:100%;border-collapse:collapse;font-size:12px}th,td{padding:5px 4px;border-bottom:1px solid var(--line);text-align:center}th{color:var(--mut);font-weight:600}
td button{background:var(--p2);border:1px solid var(--line);color:var(--tx);border-radius:7px;padding:3px 8px}
.toast{position:fixed;left:50%;bottom:calc(84px + env(safe-area-inset-bottom));transform:translateX(-50%);background:#000c;border:1px solid var(--line);padding:8px 16px;border-radius:99px;font-size:13px;z-index:9;opacity:0;transition:opacity .2s;pointer-events:none}.toast.on{opacity:1}
.kbd{font:12px ui-monospace,Menlo,monospace;background:var(--p2);border:1px solid var(--line);border-radius:5px;padding:1px 6px}
</style></head><body>
<header><div class="logo">TRO<b>J</b>AN</div><div class="dot" id="dot"></div><div class="chip" id="chipMode">connecting</div><div class="chip" id="chipCl"></div><div class="grow"></div><button id="estop">E-STOP</button></header>
<main>
<!-- DRIVE -->
<section class="tab on" id="t-drive"><div id="driveGrid">
 <div class="card" id="view"><h3>Live view</h3><canvas id="sim" width="640" height="230"></canvas></div>
 <div class="card stickWrap" id="sl"><div class="stick" id="stickL"><div class="knob"></div></div><div class="cap">Move &middot; strafe</div></div>
 <div class="card stickWrap" id="sr"><div class="stick r" id="stickR"><div class="knob"></div></div><div class="cap">Turn &middot; (up/down: stance height)</div></div>
 <div class="card" id="ctl">
  <div class="row" style="margin-bottom:8px"><div class="seg grow" id="gaitSeg"><button data-g="0">Crawl</button><button data-g="1">Trot</button></div>
   <button class="b" id="bStand">Stand</button><button class="b" id="bSit">Sit</button><button class="b" id="bRelax">Relax</button></div>
  <div class="cols c2"><div>
   <label>Speed (cycles/s) <span id="v-freq"></span></label><input type="range" id="freq" min="0.4" max="2.4" step="0.05">
   <label>Stride (deg) <span id="v-stride"></span></label><input type="range" id="stride" min="6" max="38" step="1">
   <label>Foot lift (deg) <span id="v-lift"></span></label><input type="range" id="lift" min="8" max="45" step="1"></div><div>
   <label>Stance height (0 tall - 35 low) <span id="v-height"></span></label><input type="range" id="height" min="0" max="35" step="1">
   <label>Steer trim (drift fix) <span id="v-steer"></span></label><input type="range" id="steer" min="-0.4" max="0.4" step="0.01">
   <div class="row" style="margin-top:10px"><button class="b" id="bSaveCfg">Save as default</button></div></div></div>
  <div class="cap" style="margin-top:8px">Keys: <span class="kbd">W A S D</span> move &middot; <span class="kbd">Q E</span> turn &middot; <span class="kbd">Space</span> stop &middot; <span class="kbd">Esc</span> e-stop &middot; gamepad works too</div>
 </div>
</div></section>

<!-- MOVES -->
<section class="tab" id="t-moves">
 <div class="cols c2"><div class="card"><h3>Gestures</h3><div class="grid" id="gestures"></div></div>
 <div class="card"><h3>Dances</h3><div id="dances"></div>
  <div class="row"><button class="b acc grow" id="bShow">Showtime (all dances)</button><button class="b danger" id="bStopAnim">Stop</button></div></div></div>
 <div class="card"><h3>My dances</h3><div id="udances"><span class="cap">None yet &ndash; build one in Pose Lab.</span></div></div>
</section>

<!-- FACE -->
<section class="tab" id="t-face"><div class="cols c2"><div>
 <div class="card"><h3>Moods</h3><div class="grid" id="moods"></div></div>
 <div class="card"><h3>Screens</h3><div class="grid" id="screens"></div></div>
 <div class="card"><h3>Text banner</h3><div class="row"><input type="text" id="bannerTxt" placeholder="Say something" maxlength="40" style="flex:1"><button class="b acc" id="bBanner">Show</button></div></div>
</div><div class="card"><h3>Face Studio</h3>
 <canvas id="fcv" width="128" height="64"></canvas>
 <div class="row" style="margin-top:8px"><div class="seg" id="toolSeg" style="width:150px"><button data-t="1" class="on">Draw</button><button data-t="0">Erase</button></div>
  <select id="brush" style="width:auto"><option value="1">1px</option><option value="2" selected>2px</option><option value="3">3px</option><option value="5">5px</option></select>
  <button class="b" id="fMirror">Mirror</button><button class="b" id="fClear">Clear</button><button class="b" id="fInvert">Invert</button><button class="b" id="fFlip">Flip</button></div>
 <div class="row" style="margin-top:8px"><button class="b" id="fImg">Import image</button><input type="file" id="fFile" accept="image/*" hidden><button class="b" id="fTxt">Add text</button>
  <label style="margin:0;flex:1;min-width:120px">Threshold <span id="v-thr">128</span><input type="range" id="thr" min="30" max="230" value="128"></label></div>
 <div class="row" style="margin-top:8px"><button class="b acc" id="fPrev">Preview on TROJAN</button>
  <select id="slot" style="width:auto"><option>0</option><option>1</option><option>2</option><option>3</option><option>4</option><option>5</option><option>6</option><option>7</option></select>
  <button class="b" id="fSave">Save</button><button class="b" id="fLoad">Load</button><button class="b" id="fShow">Show</button><button class="b danger" id="fDel">Delete</button></div>
 <div class="cap" id="slotInfo" style="margin-top:6px"></div>
</div></div></section>

<!-- POSE LAB -->
<section class="tab" id="t-lab"><div class="cols c2"><div class="card"><h3>Pose Lab &middot; live</h3>
 <div class="cap">Hip swing (deg, + = forward)</div><div id="hipSl"></div>
 <div class="cap" style="margin-top:8px">Foot splay (deg, 0 = down, 90 = flat)</div><div id="footSl"></div>
 <div class="row" style="margin-top:10px"><button class="b" id="lZero">Neutral</button><button class="b" id="lMirror">Link L/R</button><button class="b" id="lLive">Live: on</button><select id="lPre" style="width:auto"><option value="">Presets</option></select></div></div>
 <div class="card"><h3>Dance builder</h3>
  <div class="row"><input type="number" id="kMs" value="500" min="0" max="20000" step="50" style="width:90px"><span class="cap">ms</span>
   <select id="kFace" style="flex:1"></select><button class="b acc" id="kAdd">+ Keyframe</button></div>
  <div style="max-height:230px;overflow:auto;margin-top:8px"><table><thead><tr><th>#</th><th>ms</th><th>hips</th><th>feet</th><th></th></tr></thead><tbody id="kRows"></tbody></table></div>
  <div class="row" style="margin-top:8px"><button class="b" id="kPrev">Preview</button><button class="b danger" id="kStop">Stop</button><button class="b" id="kClear">Clear</button><span class="cap" id="kTot"></span></div>
  <div class="row" style="margin-top:8px"><input type="text" id="kName" placeholder="name" maxlength="16" style="flex:1"><button class="b acc" id="kSave">Save dance</button></div>
 </div></div></section>

<!-- TUNE -->
<section class="tab" id="t-tune"><div class="card"><h3>Per-leg tuning (grip &amp; drift)</h3>
 <div class="legg" id="legTune"></div>
 <div class="row" style="margin-top:10px"><button class="b acc" id="tSave">Save</button><button class="b" id="tNeutral">Reset trims</button></div></div>
 <div class="cols c2"><div class="card"><h3>Servo test</h3>
  <div class="row"><select id="sv" style="flex:1"></select></div>
  <label>Raw angle <span id="v-sv"></span></label><input type="range" id="svr" min="0" max="180" value="90">
  <div class="row" style="margin-top:6px"><button class="b" id="svM">-1</button><button class="b" id="svP">+1</button><button class="b" id="svC">Centre of range</button></div>
  <div class="cap" style="margin-top:8px">Moves are clamped to the tested safe range and release after 6 s.</div>
  <label>Sub-trim for this servo (deg) <span id="v-svt"></span></label><input type="range" id="svt" min="-15" max="15" value="0"></div>
 <div class="card"><h3>Motion limits</h3>
  <label>Servo speed limit (deg/s) <span id="v-maxSpeed"></span></label><input type="range" id="maxSpeed" min="100" max="800" step="10">
  <label>Dead-man timeout (ms) <span id="v-deadMs"></span></label><input type="range" id="deadMs" min="250" max="1500" step="50">
  <div class="cap" style="margin-top:8px">If the phone stops talking, TROJAN stops after the dead-man time.</div></div></div></section>

<!-- SYSTEM -->
<section class="tab" id="t-sys"><div class="cols c2"><div class="card"><h3>Status</h3><div id="sysInfo" style="font-variant-numeric:tabular-nums"></div></div>
 <div class="card"><h3>Behaviour</h3>
  <div class="row"><button class="b" id="bLife">Idle life</button><button class="b" id="bReact">Reactions</button></div>
  <label>Go to sleep after (s, 0 = never) <span id="v-sleepSec"></span></label><input type="range" id="sleepSec" min="0" max="900" step="10">
  <label>Release servos after sitting (s) <span id="v-relaxSec"></span></label><input type="range" id="relaxSec" min="2" max="60" step="1">
  <div class="row" style="margin-top:10px"><button class="b acc" id="sSave">Save settings</button><button class="b" id="sReboot">Reboot</button><button class="b danger" id="sReset">Factory reset</button></div></div></div>
 <div class="card"><h3>Connection</h3><div>Wi-Fi <b>Sesame-Controller</b> &middot; password <b>12345678</b> &middot; <span class="kbd">192.168.4.1</span> or <span class="kbd">trojan.local</span></div></div>
</section>
</main>
<nav id="nav">
 <button data-t="drive" class="on"><svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="9"/><path d="M12 7v10M7 12h10"/></svg>Drive</button>
 <button data-t="moves"><svg viewBox="0 0 24 24"><path d="M5 20l4-9 4 4 6-10"/><circle cx="19" cy="5" r="1.5"/></svg>Moves</button>
 <button data-t="face"><svg viewBox="0 0 24 24"><rect x="3" y="5" width="18" height="14" rx="3"/><circle cx="9" cy="12" r="1.5"/><circle cx="15" cy="12" r="1.5"/></svg>Face</button>
 <button data-t="lab"><svg viewBox="0 0 24 24"><path d="M9 3v6L4 19a2 2 0 002 3h12a2 2 0 002-3l-5-10V3M8 3h8"/></svg>Lab</button>
 <button data-t="tune"><svg viewBox="0 0 24 24"><path d="M4 7h10M18 7h2M4 17h4M12 17h8"/><circle cx="16" cy="7" r="2"/><circle cx="10" cy="17" r="2"/></svg>Tune</button>
 <button data-t="sys"><svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="3"/><path d="M12 2v3M12 19v3M2 12h3M19 12h3M5 5l2 2M17 17l2 2M19 5l-2 2M7 17l-2 2"/></svg>System</button>
</nav>
<div class="toast" id="toast"></div>
<script>
"use strict";
const $=s=>document.querySelector(s),$$=s=>[...document.querySelectorAll(s)];
const clamp=(v,a,b)=>Math.max(a,Math.min(b,v));
let ST={},CFG={},online=false,inflight=0;
const LEGS=["FR","RR","FL","RL"],SERVOS=["R1","R2","L1","L2","R4","R3","L3","L4"];
const SMIN=[80,0,0,80,0,0,0,0],SMAX=[180,100,100,180,160,160,160,160];
const MOODS=[[1,"Neutral"],[2,"Happy"],[3,"Sad"],[4,"Angry"],[5,"Surprised"],[6,"Sleepy"],[7,"Love"],[8,"Excited"],[9,"Wink"],[10,"Dizzy"],[11,"Suspicious"],[12,"Cool"]];
const SCREENS=[[32,"Equalizer"],[33,"Hearts"],[34,"Stars"],[35,"Spiral"],[36,"Glitch"],[37,"Notes"],[38,"Matrix"]];
const GESTURES=[["wave","Wave"],["bow","Bow"],["wiggle","Wiggle"],["stretch","Stretch"],["pushup","Push-up"],["shy","Shy"],["dead","Play dead"],["look","Look around"]];
let toastT;function toast(m){const t=$("#toast");t.textContent=m;t.classList.add("on");clearTimeout(toastT);toastT=setTimeout(()=>t.classList.remove("on"),1600)}
async function api(p,o){try{const r=await fetch(p,o);if(!r.ok)throw new Error(r.status);setOnline(true);return r}catch(e){setOnline(false);return null}}
async function jget(p){const r=await api(p);return r?r.json():null}
function setOnline(v){online=v;$("#dot").classList.toggle("on",v);if(!v)$("#chipMode").textContent="offline"}
const throttle=(f,ms)=>{let t=0,q=null;return(...a)=>{const n=Date.now();if(n-t>=ms){t=n;f(...a)}else{clearTimeout(q);q=setTimeout(()=>{t=Date.now();f(...a)},ms)}}};

/* ---------- tabs ---------- */
$("#nav").onclick=e=>{const b=e.target.closest("button");if(!b)return;$$("#nav button").forEach(x=>x.classList.toggle("on",x===b));$$(".tab").forEach(x=>x.classList.toggle("on",x.id==="t-"+b.dataset.t));if(b.dataset.t==="moves")loadDances();if(b.dataset.t==="lab")loadUserList();if(b.dataset.t==="face")slotInfo()};
function curTab(){return $("#nav .on").dataset.t}

/* ---------- state poll ---------- */
async function poll(){const s=await jget("/api/s");if(s){ST=s;
  let m=s.limp?"relaxed":s.sleep?"asleep":s.anim?("playing "+s.anim):s.pk==="seated"?"seated":(Math.abs(s.v[0])+Math.abs(s.v[1])+Math.abs(s.v[2])>.05?"walking":"standing");
  $("#chipMode").textContent=m;$("#chipCl").textContent=s.clients+(s.clients==1?" client":" clients");renderSys();}
  setTimeout(poll,s?260:1200)}

/* ---------- joysticks ---------- */
function Stick(el,cb){const k=el.querySelector(".knob");let id=null;const R=()=>el.clientWidth/2-34;
 function set(e){const b=el.getBoundingClientRect();let x=(e.clientX-b.left-b.width/2)/R(),y=(e.clientY-b.top-b.height/2)/R();const m=Math.hypot(x,y);if(m>1){x/=m;y/=m}k.style.transform=`translate(${x*R()}px,${y*R()}px)`;cb(x,-y)}
 el.addEventListener("pointerdown",e=>{id=e.pointerId;el.setPointerCapture(id);set(e)});
 el.addEventListener("pointermove",e=>{if(e.pointerId===id)set(e)});
 const end=e=>{if(e.pointerId!==id)return;id=null;k.style.transform="";cb(0,0)};
 el.addEventListener("pointerup",end);el.addEventListener("pointercancel",end)}
const stk={lx:0,ly:0,rx:0,ry:0},keys={};
Stick($("#stickL"),(x,y)=>{stk.lx=x;stk.ly=y});
Stick($("#stickR"),(x,y)=>{stk.rx=x;stk.ry=y;heightFromStick(y)});
let hBase=0,hActive=false;
const sendH=throttle(h=>{api("/api/set?height="+h.toFixed(0))},150);
function heightFromStick(y){if(!hActive&&Math.abs(y)>.3){hActive=true;hBase=+$("#height").value}if(!hActive)return;
 if(Math.abs(y)<.05){hActive=false;return}const h=clamp(hBase-y*18,0,35);$("#height").value=h;$("#v-height").textContent=h.toFixed(0);sendH(h)}
addEventListener("keydown",e=>{if(/INPUT|TEXTAREA|SELECT/.test(e.target.tagName))return;const k=e.key.toLowerCase();if(k==="escape"){estop();return}if(k===" "){e.preventDefault();keys.w=keys.s=keys.a=keys.d=keys.q=keys.e=0;return}keys[k]=1});
addEventListener("keyup",e=>{keys[e.key.toLowerCase()]=0});
addEventListener("blur",()=>{for(const k in keys)keys[k]=0});
document.addEventListener("visibilitychange",()=>{if(document.hidden){for(const k in keys)keys[k]=0;stk.lx=stk.ly=stk.rx=stk.ry=0}});
let pad={x:0,y:0,w:0},padBtn=[];
function readPad(){const g=(navigator.getGamepads?navigator.getGamepads():[])[0]||[...(navigator.getGamepads?navigator.getGamepads():[])].find(Boolean);pad={x:0,y:0,w:0};if(!g)return;
 const dz=v=>Math.abs(v)<.15?0:v;pad.x=dz(g.axes[0]||0);pad.y=-dz(g.axes[1]||0);pad.w=dz(g.axes[2]||0);
 const act=[[0,()=>stand()],[1,()=>sit()],[2,()=>play("wave")],[3,()=>play("groove")],[8,()=>estop()]];
 act.forEach(([i,f])=>{const p=g.buttons[i]&&g.buttons[i].pressed;if(p&&!padBtn[i])f();padBtn[i]=p})}
let last="0,0,0",zeros=0;
setInterval(()=>{readPad();
 let x=stk.lx+(keys.d?1:0)-(keys.a?1:0)+pad.x,y=stk.ly+(keys.w?1:0)-(keys.s?1:0)+pad.y,w=-stk.rx+(keys.q?1:0)-(keys.e?1:0)-pad.w;
 // screen right is robot right: strafe left is +y in the robot frame, turn left (counter-clockwise) is +w
 const vx=clamp(y,-1,1),vy=clamp(-x,-1,1),vw=clamp(w,-1,1);const cur=[vx,vy,vw].map(v=>v.toFixed(2)).join(",");const moving=Math.abs(vx)+Math.abs(vy)+Math.abs(vw)>.05;
 if(moving)zeros=0;
 if((moving||cur!==last||zeros<3)&&!inflight){if(!moving)zeros++;last=cur;inflight=1;api(`/api/c?x=${vx.toFixed(2)}&y=${vy.toFixed(2)}&w=${vw.toFixed(2)}`).finally(()=>inflight=0)}},70);

/* ---------- commands ---------- */
const stand=()=>{api("/api/a?a=stand");toast("Stand")},sit=()=>{api("/api/a?a=sit");toast("Sit")},estop=()=>{api("/api/a?a=estop");toast("STOP")};
const play=n=>{api("/api/play?n="+encodeURIComponent(n)).then(r=>r||toast("Could not play"))};
$("#estop").onclick=estop;$("#bStand").onclick=stand;$("#bSit").onclick=sit;$("#bRelax").onclick=()=>{api("/api/a?a=relax");toast("Servos released")};
$("#bStopAnim").onclick=()=>{api("/api/a?a=stand")};$("#bShow").onclick=()=>{api("/api/play?show=1");toast("Showtime!")};

/* ---------- sliders bound to settings ---------- */
const FMT={freq:2,stride:0,lift:0,height:0,steer:2,maxSpeed:0,deadMs:0,sleepSec:0,relaxSec:0};
const sendCfg=throttle((k,v)=>api(`/api/set?${k}=${v}`),120);
for(const k in FMT){const el=$("#"+k);if(!el)continue;el.addEventListener("input",()=>{$("#v-"+k).textContent=(+el.value).toFixed(FMT[k]);sendCfg(k,el.value)})}
function fillCfg(){for(const k in FMT){const el=$("#"+k);if(el&&CFG[k]!==undefined){el.value=CFG[k];$("#v-"+k).textContent=(+CFG[k]).toFixed(FMT[k])}}
 $$("#gaitSeg button").forEach(b=>b.classList.toggle("on",+b.dataset.g===CFG.mode));
 $("#bLife").classList.toggle("on",!!CFG.life);$("#bReact").classList.toggle("on",!!CFG.react);
 for(let i=0;i<4;i++){["ts","th","tl"].forEach(p=>{const el=$("#"+p+i);if(el){el.value=CFG[p][i];$("#v-"+p+i).textContent=(+CFG[p][i]).toFixed(2)}})}
 updSv()}
async function loadCfg(){const c=await jget("/api/cfg");if(c){CFG=c;fillCfg()}}
$("#gaitSeg").onclick=e=>{const b=e.target.closest("button");if(!b)return;CFG.mode=+b.dataset.g;api("/api/set?mode="+b.dataset.g);fillCfg()};
$("#bSaveCfg").onclick=$("#tSave").onclick=$("#sSave").onclick=async()=>{await api("/api/save");toast("Saved to TROJAN")};
$("#bLife").onclick=()=>{CFG.life=CFG.life?0:1;api("/api/set?life="+CFG.life);fillCfg()};
$("#bReact").onclick=()=>{CFG.react=CFG.react?0:1;api("/api/set?react="+CFG.react);fillCfg()};
$("#sReboot").onclick=()=>{if(confirm("Reboot TROJAN?"))api("/api/sys?a=reboot")};
$("#sReset").onclick=async()=>{if(confirm("Reset ALL settings to defaults?")){await api("/api/reset");loadCfg();toast("Defaults restored")}};
function renderSys(){const s=ST;if(!s.up&&s.up!==0)return;const u=s.up;$("#sysInfo").innerHTML=`Firmware 1.0 &middot; uptime ${Math.floor(u/3600)}h ${Math.floor(u/60)%60}m ${u%60}s<br>Clients ${s.clients} &middot; free heap ${(s.heap/1024).toFixed(0)} KB<br>Posture ${s.pk} &middot; servos ${s.limp?"released":"powered"}<br>Face ${s.face} (mood ${s.mood})`}

/* ---------- live view ---------- */
const sim=$("#sim"),sc=sim.getContext("2d");
function drawSim(){const W=sim.width,H=sim.height,cx=W/2,cy=H/2;sc.clearRect(0,0,W,H);
 sc.fillStyle="#0c1119";sc.fillRect(0,0,W,H);sc.strokeStyle="#1c2635";sc.lineWidth=1;for(let x=0;x<W;x+=40){sc.beginPath();sc.moveTo(x,0);sc.lineTo(x,H);sc.stroke()}for(let y=0;y<H;y+=40){sc.beginPath();sc.moveTo(0,y);sc.lineTo(W,y);sc.stroke()}
 const s=ST.s||[0,0,0,0],a=ST.a||[0,0,0,0];
 // robot faces right on screen so the wide canvas is used well
 const hips=[[26,26],[-26,26],[26,-26],[-26,-26]];            // FR RR FL RL: [forward,right]
 const base=[45,135,-45,-135];                                // arm direction measured from forward toward right
 const L=70;
 for(let i=0;i<4;i++){const th=(i===0?45-s[i]:i===1?135-s[i]:i===2?-45+s[i]:-135+s[i])*Math.PI/180;
  const hx=cx+hips[i][0]*2,hy=cy+hips[i][1]*1.35,fx=hx+Math.cos(th)*L*1.1,fy=hy+Math.sin(th)*L*.85;
  const lift=clamp(a[i]/45,0,1);sc.strokeStyle=lift>.15?"#35d6ff":"#ff8a3d";sc.lineWidth=7;sc.lineCap="round";sc.beginPath();sc.moveTo(hx,hy);sc.lineTo(fx,fy);sc.stroke();
  sc.fillStyle=lift>.15?"#35d6ff":"#ffb27a";sc.beginPath();sc.arc(fx,fy,7+lift*7,0,7);sc.fill();
  sc.fillStyle="#8593a8";sc.font="11px system-ui";sc.fillText(LEGS[i],fx+10,fy+4)}
 sc.fillStyle="#232f43";sc.strokeStyle="#35d6ff";sc.lineWidth=2;sc.beginPath();sc.roundRect(cx-78,cy-30,156,60,12);sc.fill();sc.stroke();
 sc.fillStyle="#35d6ff";sc.beginPath();sc.arc(cx+62,cy-9,5,0,7);sc.arc(cx+62,cy+9,5,0,7);sc.fill();
 sc.fillStyle="#8593a8";sc.font="11px system-ui";sc.fillText("front →",cx+90,cy+4);
 requestAnimationFrame(drawSim)}
drawSim();

/* ---------- moves ---------- */
GESTURES.forEach(([n,l])=>{const b=document.createElement("button");b.className="b";b.textContent=l;b.onclick=()=>play(n);$("#gestures").appendChild(b)});
async function loadDances(){const d=await jget("/api/dances");if(!d)return;
 $("#dances").innerHTML=d.builtin.map(x=>`<div class="item"><div class="n">${x.n[0].toUpperCase()+x.n.slice(1)}</div><div class="t">${x.t}s</div><button data-p="${x.n}">Play</button></div>`).join("");
 $("#udances").innerHTML=d.user.length?d.user.map(n=>`<div class="item"><div class="n">${n}</div><button data-p="u:${n}">Play</button><button data-e="${n}">Edit</button><button data-d="${n}">Delete</button></div>`).join(""):'<span class="cap">None yet &ndash; build one in Pose Lab.</span>'}
$("#t-moves").addEventListener("click",async e=>{const b=e.target.closest("button");if(!b)return;
 if(b.dataset.p)play(b.dataset.p);if(b.dataset.d&&confirm("Delete "+b.dataset.d+"?")){await api("/api/ddel?n="+b.dataset.d);loadDances()}
 if(b.dataset.e){await editDance(b.dataset.e);$$("#nav button")[3].click()}});

/* ---------- face ---------- */
MOODS.forEach(([i,n])=>{const b=document.createElement("button");b.className="b";b.textContent=n;b.onclick=()=>api("/api/mood?m="+i);$("#moods").appendChild(b)});
SCREENS.forEach(([i,n])=>{const b=document.createElement("button");b.className="b";b.textContent=n;b.onclick=()=>api("/api/screen?id="+i);$("#screens").appendChild(b)});
$("#bBanner").onclick=()=>api("/api/text?t="+encodeURIComponent($("#bannerTxt").value)+"&ms=5000");
const fc=$("#fcv"),fx=fc.getContext("2d",{willReadFrequently:true});let tool=1,mirror=false,drawing=false,lp=null;
function fclear(v){fx.fillStyle=v?"#fff":"#000";fx.fillRect(0,0,128,64)}fclear(0);
function fpt(e){const b=fc.getBoundingClientRect();return[Math.floor((e.clientX-b.left)/b.width*128),Math.floor((e.clientY-b.top)/b.height*64)]}
function dab(x,y){const s=+$("#brush").value;fx.fillStyle=tool?"#fff":"#000";const o=Math.floor(s/2);fx.fillRect(x-o,y-o,s,s);if(mirror)fx.fillRect(127-x-o,y-o,s,s)}
function line(a,b){let[x0,y0]=a,[x1,y1]=b,dx=Math.abs(x1-x0),dy=-Math.abs(y1-y0),sx=x0<x1?1:-1,sy=y0<y1?1:-1,er=dx+dy;for(;;){dab(x0,y0);if(x0===x1&&y0===y1)break;const e2=2*er;if(e2>=dy){er+=dy;x0+=sx}if(e2<=dx){er+=dx;y0+=sy}}}
fc.addEventListener("pointerdown",e=>{drawing=true;fc.setPointerCapture(e.pointerId);lp=fpt(e);dab(...lp)});
fc.addEventListener("pointermove",e=>{if(!drawing)return;const p=fpt(e);line(lp,p);lp=p});
fc.addEventListener("pointerup",()=>drawing=false);fc.addEventListener("pointercancel",()=>drawing=false);
$("#toolSeg").onclick=e=>{const b=e.target.closest("button");if(!b)return;tool=+b.dataset.t;$$("#toolSeg button").forEach(x=>x.classList.toggle("on",x===b))};
$("#fMirror").onclick=e=>{mirror=!mirror;e.target.classList.toggle("on",mirror)};
$("#fClear").onclick=()=>fclear(0);
$("#fInvert").onclick=()=>{const d=fx.getImageData(0,0,128,64);for(let i=0;i<d.data.length;i+=4){const v=d.data[i]>127?0:255;d.data[i]=d.data[i+1]=d.data[i+2]=v}fx.putImageData(d,0,0)};
$("#fFlip").onclick=()=>{const c=document.createElement("canvas");c.width=128;c.height=64;c.getContext("2d").drawImage(fc,0,0);fx.save();fx.translate(128,0);fx.scale(-1,1);fx.drawImage(c,0,0);fx.restore()};
$("#thr").oninput=e=>$("#v-thr").textContent=e.target.value;
$("#fImg").onclick=()=>$("#fFile").click();
$("#fFile").onchange=e=>{const f=e.target.files[0];if(!f)return;const im=new Image();im.onload=()=>{const c=document.createElement("canvas");c.width=128;c.height=64;const g=c.getContext("2d");g.fillStyle="#fff";g.fillRect(0,0,128,64);const r=Math.min(128/im.width,64/im.height);g.drawImage(im,(128-im.width*r)/2,(64-im.height*r)/2,im.width*r,im.height*r);
 const d=g.getImageData(0,0,128,64),t=+$("#thr").value;for(let i=0;i<d.data.length;i+=4){const l=.3*d.data[i]+.59*d.data[i+1]+.11*d.data[i+2];const v=l<t?255:0;d.data[i]=d.data[i+1]=d.data[i+2]=v}fx.putImageData(d,0,0);URL.revokeObjectURL(im.src)};im.src=URL.createObjectURL(f);e.target.value=""};
$("#fTxt").onclick=()=>{const t=prompt("Text (up to ~10 characters at size 20)");if(!t)return;fx.fillStyle="#fff";fx.font="bold 24px ui-monospace,Menlo,monospace";fx.textAlign="center";fx.textBaseline="middle";fx.fillText(t,64,32,124);
 const d=fx.getImageData(0,0,128,64);for(let i=0;i<d.data.length;i+=4){const v=d.data[i]>110?255:0;d.data[i]=d.data[i+1]=d.data[i+2]=v}fx.putImageData(d,0,0)};
function canvasHex(){const d=fx.getImageData(0,0,128,64).data;let h="";for(let y=0;y<64;y++)for(let bx=0;bx<16;bx++){let v=0;for(let b=0;b<8;b++){v=(v<<1)|(d[((y*128)+bx*8+b)*4]>127?1:0)}h+=v.toString(16).padStart(2,"0")}return h}
function hexToCanvas(h){const im=fx.createImageData(128,64);for(let y=0;y<64;y++)for(let bx=0;bx<16;bx++){const v=parseInt(h.substr((y*16+bx)*2,2),16);for(let b=0;b<8;b++){const o=((y*128)+bx*8+b)*4,on=(v>>(7-b))&1?255:0;im.data[o]=im.data[o+1]=im.data[o+2]=on;im.data[o+3]=255}}fx.putImageData(im,0,0)}
$("#fPrev").onclick=()=>api("/api/fshow",{method:"POST",headers:{"Content-Type":"text/plain"},body:canvasHex()});
$("#fSave").onclick=async()=>{const s=$("#slot").value;const r=await api("/api/fsave?slot="+s,{method:"POST",headers:{"Content-Type":"text/plain"},body:canvasHex()});toast(r?"Saved to slot "+s:"Save failed");slotInfo()};
$("#fLoad").onclick=async()=>{const r=await api("/api/fget?slot="+$("#slot").value);if(r)hexToCanvas((await r.text()).trim());else toast("Slot empty")};
$("#fShow").onclick=()=>api("/api/screen?id="+(64+ +$("#slot").value));
$("#fDel").onclick=async()=>{await api("/api/fdel?slot="+$("#slot").value);slotInfo()};
async function slotInfo(){const l=await jget("/api/flist");if(l)$("#slotInfo").textContent="Saved slots: "+(l.length?l.join(", "):"none")}

/* ---------- pose lab ---------- */
const lab={s:[0,0,0,0],a:[0,0,0,0]};let linkLR=false,liveOn=true,kfs=[];
function mkSliders(){let h="",f="";LEGS.forEach((n,i)=>{h+=`<label>${n} hip <span id="v-ls${i}">0</span></label><input type="range" id="ls${i}" min="-42" max="42" value="0">`;f+=`<label>${n} foot <span id="v-la${i}">0</span></label><input type="range" id="la${i}" min="0" max="85" value="0">`});$("#hipSl").innerHTML=h;$("#footSl").innerHTML=f;
 for(let i=0;i<4;i++){["s","a"].forEach(k=>{$("#l"+k+i).addEventListener("input",e=>{lab[k][i]=+e.target.value;if(linkLR){const j=[2,3,0,1][i];lab[k][j]=lab[k][i];}syncLab();sendLab()})})}}
function syncLab(){for(let i=0;i<4;i++){["s","a"].forEach(k=>{$("#l"+k+i).value=lab[k][i];$("#v-l"+k+i).textContent=lab[k][i]})}}
const sendLab=throttle(()=>{if(liveOn)api("/api/pose?"+lab.s.map((v,i)=>"s"+i+"="+v).join("&")+"&"+lab.a.map((v,i)=>"a"+i+"="+v).join("&"))},90);
setInterval(()=>{if(curTab()==="lab"&&liveOn)sendLab()},1100);
mkSliders();
$("#lZero").onclick=()=>{lab.s=[0,0,0,0];lab.a=[0,0,0,0];syncLab();sendLab()};
$("#lMirror").onclick=e=>{linkLR=!linkLR;e.target.classList.toggle("on",linkLR)};
$("#lLive").onclick=e=>{liveOn=!liveOn;e.target.textContent="Live: "+(liveOn?"on":"off")};
const PRE={"Stand":[[0,0,0,0],[0,0,0,0]],"Sit":[[0,0,0,0],[75,75,75,75]],"Crouch":[[0,0,0,0],[30,30,30,30]],"Bow":[[0,0,0,0],[48,0,48,0]],"Rear up":[[0,0,0,0],[0,45,0,45]],"Twist":[[26,26,-26,-26],[0,0,0,0]],"Legs forward":[[22,22,22,22],[0,0,0,0]],"High five":[[28,0,0,0],[70,0,0,0]]};
Object.keys(PRE).forEach(k=>{const o=document.createElement("option");o.textContent=k;$("#lPre").appendChild(o)});
$("#lPre").onchange=e=>{const p=PRE[e.target.value];if(!p)return;lab.s=[...p[0]];lab.a=[...p[1]];syncLab();sendLab();e.target.value=""};
const FACES=[[0,"face: keep"],...MOODS,...SCREENS];FACES.forEach(([i,n])=>{const o=document.createElement("option");o.value=i;o.textContent=n;$("#kFace").appendChild(o)});
function renderK(){let t=0;$("#kRows").innerHTML=kfs.map((k,i)=>{t+=k.ms;return`<tr><td>${i+1}</td><td>${k.ms}</td><td>${k.s.join(" ")}</td><td>${k.a.join(" ")}</td><td><button data-g="${i}">go</button><button data-u="${i}">&uarr;</button><button data-x="${i}">&times;</button></td></tr>`}).join("");$("#kTot").textContent=kfs.length?(t/1000).toFixed(1)+"s total":""}
$("#kAdd").onclick=()=>{kfs.push({ms:+$("#kMs").value||500,s:[...lab.s],a:[...lab.a],f:+$("#kFace").value});renderK()};
$("#kRows").onclick=e=>{const b=e.target.closest("button");if(!b)return;if(b.dataset.x!==undefined)kfs.splice(+b.dataset.x,1);if(b.dataset.u!==undefined){const i=+b.dataset.u;if(i>0)[kfs[i-1],kfs[i]]=[kfs[i],kfs[i-1]]}if(b.dataset.g!==undefined){const k=kfs[+b.dataset.g];lab.s=[...k.s];lab.a=[...k.a];syncLab();sendLab()}renderK()};
$("#kClear").onclick=()=>{kfs=[];renderK()};
const kText=()=>kfs.map(k=>[k.ms,...k.s,...k.a,k.f].join(" ")).join("\n");
$("#kPrev").onclick=async()=>{if(!kfs.length)return toast("Add keyframes first");const r=await api("/api/dplay",{method:"POST",headers:{"Content-Type":"text/plain"},body:kText()});if(!r)toast("Preview failed")};
$("#kStop").onclick=()=>api("/api/a?a=stand");
$("#kSave").onclick=async()=>{const n=$("#kName").value.trim();if(!n||!kfs.length)return toast("Need a name and keyframes");const r=await api("/api/dsave?n="+encodeURIComponent(n),{method:"POST",headers:{"Content-Type":"text/plain"},body:kText()});toast(r?"Saved "+n:"Save failed");loadUserList()};
async function editDance(n){const r=await api("/api/dget?n="+encodeURIComponent(n));if(!r)return;kfs=(await r.text()).split("\n").map(l=>l.trim()).filter(l=>l&&l[0]!=="#").map(l=>{const v=l.split(/\s+/).map(Number);return{ms:v[0],s:v.slice(1,5),a:v.slice(5,9),f:v[9]||0}});$("#kName").value=n;renderK()}
async function loadUserList(){renderK()}

/* ---------- tune ---------- */
let th="";LEGS.forEach((n,i)=>{th+=`<div><h4>${n}</h4><label>Stride <span id="v-ts${i}"></span></label><input type="range" id="ts${i}" min="0.5" max="1.5" step="0.01"><label>Hip offset <span id="v-th${i}"></span></label><input type="range" id="th${i}" min="-15" max="15" step="0.5"><label>Lift <span id="v-tl${i}"></span></label><input type="range" id="tl${i}" min="0.5" max="1.6" step="0.01"></div>`});$("#legTune").innerHTML=th;
for(let i=0;i<4;i++)["ts","th","tl"].forEach(p=>{const el=$("#"+p+i);el.addEventListener("input",()=>{$("#v-"+p+i).textContent=(+el.value).toFixed(2);sendCfg(p+i,el.value)})});
$("#tNeutral").onclick=async()=>{for(let i=0;i<4;i++){await api(`/api/set?ts${i}=1&th${i}=0&tl${i}=1`)}for(let i=0;i<8;i++)await api(`/api/set?st${i}=0`);loadCfg()};
SERVOS.forEach((n,i)=>{const o=document.createElement("option");o.value=i;o.textContent=n+" (idx "+i+")";$("#sv").appendChild(o)});
function updSv(){const i=+$("#sv").value,r=$("#svr");r.min=SMIN[i];r.max=SMAX[i];r.value=clamp(+r.value,SMIN[i],SMAX[i]);$("#v-sv").textContent=r.value;if(CFG.st){$("#svt").value=CFG.st[i];$("#v-svt").textContent=CFG.st[i]}}
const sendSv=throttle(()=>api(`/api/raw?i=${$("#sv").value}&v=${$("#svr").value}`),90);
setInterval(()=>{if(curTab()==="tune"&&svActive)sendSv()},1500);let svActive=false;
$("#sv").onchange=()=>{svActive=false;updSv()};
$("#svr").oninput=()=>{svActive=true;$("#v-sv").textContent=$("#svr").value;sendSv()};
$("#svM").onclick=()=>{$("#svr").value=+$("#svr").value-1;svActive=true;$("#v-sv").textContent=$("#svr").value;sendSv()};
$("#svP").onclick=()=>{$("#svr").value=+$("#svr").value+1;svActive=true;$("#v-sv").textContent=$("#svr").value;sendSv()};
$("#svC").onclick=()=>{const i=+$("#sv").value;$("#svr").value=(SMIN[i]+SMAX[i])/2;svActive=true;$("#v-sv").textContent=$("#svr").value;sendSv()};
$("#svt").oninput=()=>{const i=+$("#sv").value;$("#v-svt").textContent=$("#svt").value;CFG.st[i]=+$("#svt").value;sendCfg("st"+i,$("#svt").value)};

loadCfg();poll();
</script></body></html>)TROJANHTML";
