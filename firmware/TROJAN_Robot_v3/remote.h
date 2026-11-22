#pragma once
// The TROJAN remote: one self-contained page served from flash (no internet on TROJAN's Wi-Fi, so no web
// fonts or CDNs). Four tabs; Drive is the default. Advanced tools live in folded sections under More.
static const char REMOTE_HTML[] PROGMEM = R"TROJANHTML(<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#E9EDF1"><title>TROJAN remote</title>
<style>
:root{--paper:#E9EDF1;--panel:#fff;--ink:#1D2430;--mut:#5D6878;--line:#CDD4DC;--blue:#1F5FD6;--blue2:#E3ECFC;--red:#D7263D;--ok:#1E9E5A;
 --f:ui-rounded,"SF Pro Rounded","Segoe UI",system-ui,-apple-system,Roboto,sans-serif;--stick:min(210px,56vw,26.5vh)}
*{box-sizing:border-box}
html{-webkit-text-size-adjust:100%}
body{margin:0;background:var(--paper);color:var(--ink);font:16px/1.45 var(--f);overscroll-behavior:none}
button,input,select{font:inherit;color:inherit}
button{touch-action:manipulation;cursor:pointer;-webkit-tap-highlight-color:transparent}
:focus-visible{outline:3px solid var(--blue);outline-offset:2px}
.top{position:sticky;top:0;z-index:5;display:flex;align-items:center;gap:12px;background:var(--paper);border-bottom:1px solid var(--line);
 padding:calc(8px + env(safe-area-inset-top)) max(16px,env(safe-area-inset-right)) 8px max(16px,env(safe-area-inset-left))}
.status{display:flex;align-items:center;gap:8px;flex:1;min-width:0}
#stText{font-weight:600;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.dot{flex:none;width:10px;height:10px;border-radius:50%;background:var(--red)}.dot.on{background:var(--ok)}
.stop{flex:none;min-width:96px;min-height:44px;border:0;border-radius:12px;background:var(--red);color:#fff;font-weight:800;font-size:17px}
.stop:active{filter:brightness(.88)}
.offline{margin:0;padding:10px 16px;background:#FDECEE;color:#8A1424;border-bottom:1px solid #F3C4CB;font-size:15px}
nav{position:fixed;left:0;right:0;bottom:0;z-index:5;display:flex;background:var(--panel);border-top:1px solid var(--line);padding-bottom:env(safe-area-inset-bottom)}
nav button{flex:1;display:flex;flex-direction:column;align-items:center;gap:2px;min-height:56px;padding:8px 4px;background:none;border:0;font-size:13px;font-weight:600;color:var(--mut)}
nav svg{width:24px;height:24px;fill:none;stroke:currentColor;stroke-width:1.8;stroke-linecap:round;stroke-linejoin:round}
nav button.on{color:var(--blue)}
main{max-width:1040px;margin:0 auto;padding:16px max(16px,env(safe-area-inset-right)) calc(88px + env(safe-area-inset-bottom)) max(16px,env(safe-area-inset-left))}
.tab{display:none}.tab.on{display:block}
h2{font-size:19px;margin:28px 0 10px}h2:first-child{margin-top:4px}
h3{font-size:16px;margin:18px 0 8px}
.note{margin:-4px 0 12px;color:var(--mut);font-size:15px}
.btn{min-height:48px;padding:10px 14px;border:1px solid var(--line);border-radius:12px;background:var(--panel);font-weight:600;font-size:16px}
.btn:active{background:var(--blue2)}
.btn.primary{background:var(--blue);border-color:var(--blue);color:#fff}
.btn.danger{color:var(--red);border-color:#EFB3BC}
.btn[aria-pressed=true]{background:var(--blue2);border-color:var(--blue);color:var(--blue)}
.row{display:flex;flex-wrap:wrap;gap:8px;margin-top:12px}.row>.btn{flex:1 1 130px}
.tiles{display:grid;gap:8px;grid-template-columns:repeat(auto-fill,minmax(136px,1fr))}
.tiles .btn{min-height:56px}
.seg{display:flex;gap:4px;padding:4px;background:#DCE2E8;border-radius:14px}
.seg button{flex:1;min-height:44px;border:0;border-radius:10px;background:none;font-weight:600;color:var(--mut)}
.seg button.on{background:var(--panel);color:var(--ink);box-shadow:0 1px 2px rgba(29,36,48,.2)}
/* drive */
#drive{display:grid;gap:12px;grid-template-areas:"face" "pad" "act" "speed" "quick" "keys"}
.bezel{position:relative;grid-area:face;justify-self:center;width:min(100%,330px,33vh);padding:10px;border-radius:22px;background:var(--ink)}
.mirror{display:block;width:100%;aspect-ratio:2/1;background:#000;border-radius:8px;image-rendering:pixelated}
.legsview{display:block;width:100%;aspect-ratio:2/1;background:#0C1118;border-radius:8px}
#scr.face .legsview,#scr:not(.face) .mirror{display:none}
.vsw{display:flex;gap:2px;width:max-content;margin:8px auto 0;padding:2px;background:rgba(255,255,255,.12);border-radius:9px}
.vsw button{min-height:34px;padding:2px 10px;border:0;border-radius:7px;background:none;color:#C3CCD8;font-size:13px;font-weight:600}
.vsw button.on{background:#fff;color:var(--ink)}
.pad{grid-area:pad;display:flex;justify-content:center;gap:24px}
.stickbox{display:flex;flex-direction:column;align-items:center;gap:8px}
.stick{position:relative;width:var(--stick);height:var(--stick);border-radius:50%;background:radial-gradient(circle,#F7F9FB,#DCE2E9);border:1px solid var(--line);touch-action:none}
.stick::before{content:"";position:absolute;inset:20%;border-radius:50%;border:1px dashed #B5C0CC}
.knob{position:absolute;left:33%;top:33%;width:34%;height:34%;border-radius:50%;background:var(--blue);box-shadow:0 6px 14px rgba(31,95,214,.35);pointer-events:none}
.hint{margin:0;font-size:14px;color:var(--mut);text-align:center}
@media (orientation:portrait) and (max-height:700px){#drive{gap:8px}.stickbox{gap:4px}}
#boxR{display:none}#drive.two #boxR{display:flex}#drive.two{--stick:min(170px,41vw)}
.act{grid-area:act;display:grid;grid-template-columns:1fr 1fr 1fr;gap:8px}
#boost{border-color:var(--ink);background:var(--ink);color:#fff;user-select:none;-webkit-user-select:none;touch-action:none}
#boost.on{background:var(--blue);border-color:var(--blue)}
#speedSeg{grid-area:speed}
.quick{grid-area:quick;display:flex;gap:8px;overflow-x:auto;padding-bottom:2px;scrollbar-width:none}.quick::-webkit-scrollbar{display:none}
.quick .btn{flex:none;min-height:44px}
.keys{grid-area:keys;display:none;margin:0;font-size:14px;color:var(--mut)}
@media (pointer:fine){.keys{display:block}}
@media (min-width:600px) and (orientation:portrait){.bezel{width:min(100%,480px,33vh)}}
/* laptop and big tablets: tabs move into the top bar, the live display gets the left column */
@media (min-width:860px) and (min-height:560px){
 nav{position:static;flex:none;background:none;border:0;padding:0}
 nav button{flex-direction:row;gap:6px;min-height:44px;padding:6px 12px;font-size:15px}
 nav svg{width:20px;height:20px}
 main{padding-bottom:32px}
 .toast{bottom:24px}
 #drive{grid-template-columns:minmax(0,1.35fr) minmax(300px,1fr);grid-template-areas:"face act" "face speed" "face quick" "face pad" "face keys";column-gap:32px;row-gap:16px;align-items:start;align-content:start}
 .bezel{width:100%;max-width:640px}
 .pad{justify-content:flex-start}
 .quick{flex-wrap:wrap;overflow:visible}
}
@media (orientation:landscape) and (max-height:540px){
 :root{--stick:min(190px,50vh)}
 #drive.two{--stick:min(170px,46vh)}
 .top{padding-top:calc(4px + env(safe-area-inset-top));padding-bottom:4px}
 nav{position:static;flex:none;background:none;border:0;padding:0}
 nav button{flex-direction:row;gap:6px;min-height:44px;padding:6px 10px;font-size:14px}
 nav svg{width:20px;height:20px}
 main{padding-top:10px;padding-bottom:calc(12px + env(safe-area-inset-bottom))}
 #speedSeg button{font-size:15px;padding:0 4px}
 #drive{grid-template-columns:auto minmax(0,1fr) minmax(132px,28%);grid-template-areas:"L face act" "L speed act" "L quick act";align-items:center;column-gap:20px}
 #drive.two{grid-template-columns:auto minmax(0,1fr) auto;grid-template-areas:"L face R" "L speed R" "L act R"}
 #drive.two .quick{display:none}
 .pad{display:contents}#boxL{grid-area:L}#boxR{grid-area:R}
 .bezel{width:min(100%,280px,calc((100vh - 236px) * 2));padding:8px;border-radius:16px}
 .act{grid-template-columns:1fr}
 #drive{row-gap:8px}
 #drive.two .act{grid-template-columns:1fr 1fr 1fr}
 #drive.two .bezel{width:min(100%,280px,calc((100vh - 262px) * 2))}
 .act .btn{min-height:44px;padding:8px 6px;font-size:15px}
 .keys{display:none}
 .toast{bottom:16px}
}
@media (orientation:landscape) and (max-height:370px),(orientation:portrait) and (max-height:760px){.quick{display:none}}
/* lists and sections */
.list{display:grid;gap:8px}
.item{display:flex;flex-wrap:wrap;align-items:center;gap:8px 10px;padding:8px 8px 8px 14px;background:var(--panel);border:1px solid var(--line);border-radius:12px}
.item .n{flex:1 1 110px;font-weight:600;min-width:0;overflow-wrap:anywhere}.item .t{color:var(--mut);font-size:14px;font-variant-numeric:tabular-nums}
.item button{min-height:40px;padding:6px 14px;border:1px solid var(--line);border-radius:10px;background:var(--paper);font-weight:600}
details{background:var(--panel);border:1px solid var(--line);border-radius:14px;margin-bottom:10px}
summary{display:flex;align-items:center;gap:12px;min-height:60px;padding:12px 16px;font-weight:700;cursor:pointer;list-style:none}
summary::-webkit-details-marker{display:none}
summary span{flex:1}summary small{display:block;font-weight:400;font-size:14px;color:var(--mut)}
summary::after{content:"";flex:none;width:9px;height:9px;border-right:2px solid var(--mut);border-bottom:2px solid var(--mut);transform:rotate(45deg);margin-top:-4px}
details[open] summary::after{transform:rotate(-135deg);margin-top:4px}
.body{padding:0 16px 16px}
label.f{display:flex;justify-content:space-between;gap:8px;margin:14px 0 2px}
label.f output{color:var(--mut);font-variant-numeric:tabular-nums}
input[type=range]{display:block;width:100%;height:32px;margin:0;accent-color:var(--blue)}
.ends{display:flex;justify-content:space-between;font-size:13px;color:var(--mut)}
input[type=text],input[type=number],input[type=password],select{min-height:44px;width:100%;padding:8px 12px;border:1px solid var(--line);border-radius:10px;background:var(--panel)}
.inline{display:flex;gap:8px;align-items:center}.inline>*{flex:1}.inline>.btn{flex:none}
.legs{display:grid;gap:4px 16px;grid-template-columns:repeat(auto-fit,minmax(170px,1fr))}
.legs h3{margin:14px 0 0}
table{width:100%;border-collapse:collapse;font-size:14px}th,td{padding:6px 4px;border-bottom:1px solid var(--line);text-align:center}th{color:var(--mut);font-weight:600}
td button{min-height:34px;padding:4px 9px;margin:1px;border:1px solid var(--line);border-radius:8px;background:var(--paper)}
.scroll{max-height:240px;overflow:auto;margin-top:10px}
#fcv{display:block;width:100%;max-width:512px;aspect-ratio:2/1;background:#000;border-radius:8px;image-rendering:pixelated;touch-action:none}
.cols{display:grid;gap:0 24px}
@media (min-width:760px){.cols{grid-template-columns:1fr 1fr}}
.info{margin:0;line-height:1.7;font-variant-numeric:tabular-nums}
.toast{position:fixed;left:50%;bottom:calc(72px + env(safe-area-inset-bottom));transform:translateX(-50%);z-index:9;max-width:calc(100% - 32px);padding:10px 18px;border-radius:12px;background:var(--ink);color:#fff;font-size:15px;opacity:0;transition:opacity .2s;pointer-events:none}
.toast.on{opacity:1}
@media (prefers-reduced-motion:reduce){*{transition:none!important}}
</style></head><body>
<header class="top">
 <div class="status" role="status"><span class="dot" id="dot"></span><span id="stText">Connecting</span></div>
 <nav id="nav" aria-label="Sections">
  <button data-t="drive" class="on" aria-current="page"><svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="9"/><circle cx="12" cy="12" r="3"/></svg>Drive</button>
  <button data-t="play"><svg viewBox="0 0 24 24"><path d="M7 5l12 7-12 7z"/></svg>Play</button>
  <button data-t="face"><svg viewBox="0 0 24 24"><rect x="3" y="5" width="18" height="14" rx="3"/><path d="M9 11v2M15 11v2"/></svg>Face</button>
  <button data-t="more"><svg viewBox="0 0 24 24"><path d="M4 7h16M4 12h16M4 17h16"/></svg>More</button>
 </nav>
 <button class="stop" id="estop">Stop</button>
</header>
<p class="offline" id="offline" hidden>Can't reach TROJAN. Join the <b>Sesame-Controller</b> Wi-Fi on this device, then come back to this page.</p>
<main>
<section class="tab on" id="t-drive"><div id="drive">
 <div class="bezel" id="scr"><canvas class="legsview" width="640" height="320" aria-label="TROJAN's legs, live, seen from above"></canvas><canvas class="mirror" width="128" height="64" aria-label="TROJAN's face, live"></canvas>
  <div class="vsw" role="group" aria-label="Display"><button data-v="legs">Legs</button><button data-v="face">Face</button></div></div>
 <div class="pad">
  <div class="stickbox" id="boxL"><div class="stick" id="stickL" aria-label="Drive joystick"><div class="knob"></div></div><p class="hint" id="hintL"></p></div>
  <div class="stickbox" id="boxR"><div class="stick" id="stickR" aria-label="Turn joystick"><div class="knob"></div></div><p class="hint">Rotate</p></div>
 </div>
 <div class="act"><button class="btn" id="bStand">Stand up</button><button class="btn" id="bSit">Sit down</button><button class="btn" id="boost">Hold to run</button></div>
 <div class="seg" id="speedSeg" role="group" aria-label="Walking speed"><button data-p="0">Slow</button><button data-p="1">Normal</button><button data-p="2">Fast</button></div>
 <div class="quick" id="quick" aria-label="Quick moves"></div>
 <p class="keys">Keyboard: W and S walk, A and D side-step, Q and E or the arrow keys turn, Shift runs, Space lets go, Esc stops. Keys 1 to 0 play the quick moves.</p>
</div></section>

<section class="tab" id="t-play">
 <h2>Tricks</h2><div class="tiles" id="gestures"></div>
 <div class="row"><button class="btn primary" id="bSurprise">Surprise me</button></div>
 <label class="f" for="size">How big tricks and dances move <output id="v-size"></output></label><input type="range" id="size" min="0.4" max="1" step="0.05">
 <h2>Moving tricks</h2><p class="note">These walk around, so give TROJAN about a metre of space. Touching the joystick ends them.</p><div class="tiles" id="dtricks"></div>
 <h2>Dances</h2><div class="list" id="dances"></div>
 <label class="f" for="tempo">Dance speed <output id="v-tempo"></output></label><input type="range" id="tempo" min="0.6" max="1.6" step="0.05">
 <div class="row"><button class="btn primary" id="bShow">Play all dances</button><button class="btn" id="bStopAnim">Stop dancing</button></div>
 <h2>My dances</h2><div class="list" id="udances"></div>
 <div class="row"><button class="btn" id="dImp">Open a dance file</button><input type="file" id="dFile" accept=".txt,text/plain" hidden></div>
</section>

<section class="tab" id="t-face">
 <div class="bezel" style="margin:4px auto 0"><canvas class="mirror" width="128" height="64" aria-label="TROJAN's face, live"></canvas></div>
 <h2>Moods</h2><div class="tiles" id="moods"></div>
 <h2>Animations</h2><div class="tiles" id="screens"></div>
 <h2>Show a message</h2><div class="inline"><input type="text" id="bannerTxt" placeholder="Type a short message" maxlength="40" aria-label="Message"><button class="btn primary" id="bBanner">Show it</button></div>
</section>

<section class="tab" id="t-more">
 <details id="dWalk"><summary><span>Walking style<small>How TROJAN steps, how fast and how high</small></span></summary><div class="body">
  <div class="seg" id="gaitSeg" role="group" aria-label="Walking style"><button data-g="0">Crawl</button><button data-g="1">Trot</button><button data-g="2">Run</button></div>
  <p class="note" id="gaitInfo" style="margin:10px 0 0"></p>
  <div class="cols"><div>
   <label class="f" for="freq">Steps per second <output id="v-freq"></output></label><input type="range" id="freq" min="0.4" max="2.4" step="0.05">
   <label class="f" for="stride">Step length <output id="v-stride"></output></label><input type="range" id="stride" min="6" max="38" step="1">
  </div><div>
   <label class="f" for="lift">Foot lift <output id="v-lift"></output></label><input type="range" id="lift" min="8" max="45" step="1">
   <label class="f" for="height">Body height <output id="v-height"></output></label><input type="range" id="height" min="0" max="35" step="1"><div class="ends"><span>Tall</span><span>Low</span></div>
  </div></div>
  <div class="row"><button class="btn primary save">Save these settings</button></div>
 </div></details>

 <details id="dDrift"><summary><span>Fix drifting<small>Make TROJAN walk in a straight line</small></span></summary><div class="body">
  <p class="note" style="margin:0">Walk TROJAN straight ahead. If it curves off to the right, move the slider toward "Steer left" a little and try again.</p>
  <label class="f" for="steer">Steering correction <output id="v-steer"></output></label><input type="range" id="steer" min="-0.4" max="0.4" step="0.01"><div class="ends"><span>Steer right</span><span>Steer left</span></div>
  <h3>Per-leg fine tuning</h3><p class="note">For a leg that slips or steps short. 1.00 means unchanged.</p>
  <div class="legs" id="legTune"></div>
  <div class="row"><button class="btn primary save">Save these settings</button><button class="btn" id="tNeutral">Reset leg tuning</button></div>
 </div></details>

 <details id="dCtl"><summary><span>Driving controls<small>One joystick or two</small></span></summary><div class="body">
  <p class="note" style="margin:0">One joystick works like a remote-control car: up walks, sideways turns. With two joysticks the left one moves (walk and side-step) and the right one rotates; push the right one up or down to raise or lower the body. Sideways phones and laptops start with two.</p>
  <div class="row"><button class="btn" id="bTwo" aria-pressed="false">Use two joysticks</button></div>
 </div></details>

 <details id="dRoute"><summary><span>Record a route<small>Drive a path once, then replay it</small></span></summary><div class="body">
  <p class="note" style="margin:0">Press Record, drive, then press Stop recording. The route is kept on this device.</p>
  <div class="row"><button class="btn" id="mRec">Record</button><button class="btn" id="mPlay">Replay</button></div>
  <p class="note" id="mInfo" style="margin:10px 0 0"></p>
 </div></details>

 <details id="dLab"><summary><span>Pose lab and dance builder<small>Set each leg by hand and make your own dances</small></span></summary><div class="body">
  <div class="bezel" style="width:100%;max-width:520px;margin-top:4px"><canvas class="legsview" width="640" height="320" aria-label="TROJAN's legs, live, seen from above"></canvas></div>
  <div class="cols"><div><h3>Hips (forward is positive)</h3><div id="hipSl"></div></div><div><h3>Feet (0 down, 90 flat, 120 paw up)</h3><div id="footSl"></div></div></div>
  <div class="row"><button class="btn" id="lCopy">Copy TROJAN's pose</button><button class="btn" id="lZero">Reset pose</button><button class="btn" id="lMirror" aria-pressed="false">Move left and right together</button><button class="btn" id="lLive" aria-pressed="true">Live on robot</button></div>
  <label class="f" for="lPre">Start from a preset</label><select id="lPre"><option value="">Choose a preset</option></select>
  <h3>Dance builder</h3><p class="note">Set a pose above, choose how long to move to it, then add it as a step.</p>
  <div class="inline"><input type="number" id="kMs" value="500" min="0" max="20000" step="50" aria-label="Milliseconds to reach this pose"><select id="kFace" aria-label="Face during this step"></select><button class="btn primary" id="kAdd">Add step</button></div>
  <div class="scroll"><table><thead><tr><th>Step</th><th>ms</th><th>Hips</th><th>Feet</th><th></th></tr></thead><tbody id="kRows"></tbody></table></div>
  <p class="note" id="kTot" style="margin:8px 0 0"></p>
  <div class="row"><button class="btn primary" id="kPrev">Try it on TROJAN</button><button class="btn" id="kStop">Stop</button><button class="btn danger" id="kClear">Clear steps</button></div>
  <div class="inline" style="margin-top:12px"><input type="text" id="kName" placeholder="Dance name" maxlength="16" aria-label="Dance name"><button class="btn primary" id="kSave">Save dance</button></div>
 </div></details>

 <details id="dStudio"><summary><span>Face studio<small>Draw your own face for TROJAN's screen</small></span></summary><div class="body">
  <canvas id="fcv" width="128" height="64" aria-label="Drawing canvas"></canvas>
  <div class="row"><div class="seg" id="toolSeg" style="flex:1 1 180px"><button data-t="1" class="on">Draw</button><button data-t="0">Erase</button></div>
   <select id="brush" style="flex:0 1 120px" aria-label="Brush size"><option value="1">Thin</option><option value="2" selected>Medium</option><option value="3">Thick</option><option value="5">Extra thick</option></select></div>
  <div class="row"><button class="btn" id="fMirror" aria-pressed="false">Mirror drawing</button><button class="btn" id="fInvert">Invert</button><button class="btn" id="fFlip">Flip</button><button class="btn danger" id="fClear">Clear</button></div>
  <div class="row"><button class="btn" id="fImg">Use a picture</button><input type="file" id="fFile" accept="image/*" hidden><button class="btn" id="fTxt">Add text</button></div>
  <label class="f" for="thr">Picture brightness cut-off <output id="v-thr">128</output></label><input type="range" id="thr" min="30" max="230" value="128">
  <div class="row"><button class="btn primary" id="fPrev">Show on TROJAN now</button></div>
  <h3>Saved faces</h3>
  <div class="inline"><select id="slot" aria-label="Save slot"><option value="0">Slot 1</option><option value="1">Slot 2</option><option value="2">Slot 3</option><option value="3">Slot 4</option><option value="4">Slot 5</option><option value="5">Slot 6</option><option value="6">Slot 7</option><option value="7">Slot 8</option></select></div>
  <div class="row"><button class="btn" id="fSave">Save to slot</button><button class="btn" id="fLoad">Open slot</button><button class="btn" id="fShow">Show slot</button><button class="btn danger" id="fDel">Delete slot</button></div>
  <p class="note" id="slotInfo" style="margin:10px 0 0"></p>
 </div></details>

 <details id="dServo"><summary><span>Servo test<small>Move one motor at a time, set motor offsets and limits</small></span></summary><div class="body">
  <select id="sv" aria-label="Motor"></select>
  <label class="f" for="svr">Motor angle <output id="v-sv"></output></label><input type="range" id="svr" min="0" max="180" value="90">
  <div class="row"><button class="btn" id="svM">Minus 1</button><button class="btn" id="svP">Plus 1</button><button class="btn" id="svC">Middle</button></div>
  <p class="note" style="margin:10px 0 0">Angles stay inside the tested safe range, and the motor lets go after 6 seconds.</p>
  <label class="f" for="svt">Offset for this motor <output id="v-svt"></output></label><input type="range" id="svt" min="-15" max="15" value="0">
  <label class="f" for="maxSpeed">Motor speed limit <output id="v-maxSpeed"></output></label><input type="range" id="maxSpeed" min="100" max="800" step="10">
  <label class="f" for="deadMs">Stop if the remote goes quiet for (ms) <output id="v-deadMs"></output></label><input type="range" id="deadMs" min="250" max="1500" step="50">
  <div class="row"><button class="btn primary save">Save these settings</button></div>
 </div></details>

 <details id="dAll"><summary><span>All motors at once<small>Set every one of the 8 motor angles by hand</small></span></summary><div class="body">
  <p class="note" style="margin:0">Angles stay inside the tested safe range. Each motor goes back to normal 6 seconds after you stop touching it.</p>
  <div class="cols" id="allSv"></div>
  <div class="row"><button class="btn" id="amFill">Read current angles</button><button class="btn" id="amFree">Let go of all motors</button></div>
 </div></details>

 <details id="dLife"><summary><span>Behaviour<small>Idle moves, greetings and sleep</small></span></summary><div class="body">
  <div class="row" style="margin-top:0"><button class="btn" id="bLife" aria-pressed="true">Moves on its own when idle</button><button class="btn" id="bReact" aria-pressed="true">Greets new connections</button></div>
  <label class="f" for="sleepSec">Fall asleep after (seconds, 0 never) <output id="v-sleepSec"></output></label><input type="range" id="sleepSec" min="0" max="900" step="10">
  <label class="f" for="relaxSec">Turn motors off after sitting for (seconds) <output id="v-relaxSec"></output></label><input type="range" id="relaxSec" min="2" max="60" step="1">
  <div class="row"><button class="btn primary save">Save these settings</button></div>
 </div></details>

 <details id="dWifi"><summary><span>Wi-Fi<small>One device at a time, change the password</small></span></summary><div class="body">
  <p class="note" style="margin:0">TROJAN's Wi-Fi is <b>Sesame-Controller</b>, at 192.168.4.1 or trojan.local. Only one phone or laptop can be connected at a time. To let someone else drive, disconnect this device from that Wi-Fi. TROJAN shows the password on its screen when it starts.</p>
  <label class="f" for="wPass">New password (8 to 63 characters)</label>
  <form class="inline" id="wForm"><input type="password" id="wPass" autocomplete="new-password"><button class="btn primary">Change password</button></form>
 </div></details>

 <details id="dSys"><summary><span>System<small>Status, restart and reset</small></span></summary><div class="body">
  <p class="info" id="sysInfo"></p>
  <div class="row"><button class="btn" id="bRelax">Turn motors off</button><button class="btn" id="sReboot">Restart TROJAN</button><button class="btn danger" id="sReset">Reset all settings</button></div>
 </div></details>
</section>
</main>
<div class="toast" id="toast" role="status"></div>
<script>
"use strict";
const $=s=>document.querySelector(s),$$=s=>[...document.querySelectorAll(s)];
const clamp=(v,a,b)=>Math.max(a,Math.min(b,v));
let ST={},CFG={},online=false,inflight=0,offT=0,driving=false;
const LEGS=["Front right","Back right","Front left","Back left"],SERVOS=["R1","R2","L1","L2","R4","R3","L3","L4"];
const SMIN=[80,0,0,80,0,0,0,0],SMAX=[180,100,100,180,160,160,160,160];
const MOODS=[[1,"Neutral"],[2,"Happy"],[3,"Sad"],[4,"Angry"],[5,"Surprised"],[6,"Sleepy"],[7,"Love"],[8,"Excited"],[9,"Wink"],[10,"Dizzy"],[11,"Suspicious"],[12,"Cool"],[13,"Curious"],[14,"Scared"],[15,"Bored"],[16,"Laugh"],[17,"Determined"]];
const SCREENS=[[32,"Equalizer"],[33,"Hearts"],[34,"Stars"],[35,"Spiral"],[36,"Glitch"],[37,"Music notes"],[38,"Matrix"],[40,"Heart monitor"]];
const GAITS=["Crawl: one leg up at a time, three feet always on the ground. Slow and very steady.","Trot: legs move in diagonal pairs. About twice as fast and still balanced.","Run: a fast trot with longer steps. You can also hold the Run button in any style."];
const RST={1:"power on",3:"restart",4:"crash",5:"crash",6:"crash",7:"crash",9:"power dip, check the battery",12:"restart"};
let MOVES={tricks:[],drive:[],builtin:[],user:[]},LABEL={startup:"wake-up",preview:"your dance"};
let toastT;function toast(m){const t=$("#toast");t.textContent=m;t.classList.add("on");clearTimeout(toastT);toastT=setTimeout(()=>t.classList.remove("on"),1800)}
// Any answer, even an error status, means TROJAN is reachable; only a network failure or a timeout means
// offline. The timeout stops one hung drive request from blocking every later one (see `inflight`).
async function api(p,o={}){const c=new AbortController(),t=setTimeout(()=>c.abort(),p.startsWith("/api/c?")?900:4000);
 try{const r=await fetch(p,{...o,signal:c.signal});setOnline(true);return r.ok?r:null}catch(e){setOnline(false);return null}finally{clearTimeout(t)}}
async function jget(p){const r=await api(p);return r?r.json():null}
function setOnline(v){online=v;$("#dot").classList.toggle("on",v);
 if(v){clearTimeout(offT);offT=0;$("#offline").hidden=true}
 else if(!offT)offT=setTimeout(()=>{offT=0;if(!online){$("#offline").hidden=false;$("#stText").textContent="Not connected"}},2000)}
const throttle=(f,ms)=>{let t=0,q=null;return(...a)=>{const n=Date.now();if(n-t>=ms){t=n;f(...a)}else{clearTimeout(q);q=setTimeout(()=>{t=Date.now();f(...a)},ms)}}};

/* ---------- tabs ---------- */
function showTab(t){$$("#nav button").forEach(b=>{const on=b.dataset.t===t;b.classList.toggle("on",on);b.setAttribute("aria-current",on?"page":"false")});
 $$(".tab").forEach(x=>x.classList.toggle("on",x.id==="t-"+t));if(t==="play")loadDances();if(t==="more")slotInfo();scrollTo(0,0)}
$("#nav").onclick=e=>{const b=e.target.closest("button");if(b)showTab(b.dataset.t)};
function curTab(){return $("#nav .on").dataset.t}
function openSection(id){showTab("more");const d=$(id);d.open=true;d.scrollIntoView({block:"start"})}

/* ---------- state ---------- */
function words(s){if(s.limp)return"Resting, motors off";if(s.sleep)return"Asleep";if(s.anim)return"Playing "+(LABEL[s.anim]||s.anim.replace(/^u:/,""));
 if(s.trick)return LABEL[s.trick]||s.trick;if(s.run)return"Running";if(s.pk==="seated")return"Sitting";
 return Math.abs(s.v[0])+Math.abs(s.v[1])+Math.abs(s.v[2])>.05?"Walking":"Standing"}
async function poll(){const s=await jget("/api/s");if(s){ST=s;$("#stText").textContent=words(s);renderSys()}setTimeout(poll,!s?1200:curTab()==="drive"&&view==="legs"?200:300)}
function renderSys(){const s=ST;if(s.up===undefined)return;const u=s.up;
 $("#sysInfo").innerHTML=`Firmware ${s.fw||"?"}<br>On for ${Math.floor(u/3600)} h ${Math.floor(u/60)%60} min<br>Motors ${s.limp?"off":"on"}<br>Free memory ${(s.heap/1024).toFixed(0)} KB<br>Last start: ${RST[s.rst]||"code "+s.rst}`}

/* ---------- joysticks, keys, gamepad ---------- */
function Stick(el,cb){const k=el.querySelector(".knob");let id=null;const R=()=>(el.clientWidth-k.offsetWidth)/2;
 function set(e){const b=el.getBoundingClientRect();let x=(e.clientX-b.left-b.width/2)/R(),y=(e.clientY-b.top-b.height/2)/R();const m=Math.hypot(x,y);if(m>1){x/=m;y/=m}k.style.transform=`translate(${x*R()}px,${y*R()}px)`;cb(x,-y)}
 el.addEventListener("pointerdown",e=>{id=e.pointerId;el.setPointerCapture(id);set(e)});
 el.addEventListener("pointermove",e=>{if(e.pointerId===id)set(e)});
 const end=e=>{if(e.pointerId!==id)return;id=null;k.style.transform="";cb(0,0)};
 el.addEventListener("pointerup",end);el.addEventListener("pointercancel",end)}
const stk={lx:0,ly:0,rx:0,ry:0},keys={};
Stick($("#stickL"),(x,y)=>{stk.lx=x;stk.ly=y});
Stick($("#stickR"),(x,y)=>{stk.rx=x;stk.ry=y;heightFromStick(y)});
// right stick up/down sets body height (as in 2.0), only after a clear push so turning does not change it
let hBase=0,hActive=false;
const sendH=throttle(h=>api("/api/set?height="+h.toFixed(0)),150);
function heightFromStick(y){if(!hActive&&Math.abs(y)>.3){hActive=true;hBase=+$("#height").value}if(!hActive)return;
 if(Math.abs(y)<.05){hActive=false;return}const h=clamp(hBase-y*18,0,35);$("#height").value=h;$("#v-height").textContent=h.toFixed(0);sendH(h)}
// Two joysticks by default when both thumbs are free (sideways phone, laptop); a choice made in More is remembered.
const wide=matchMedia("(orientation:landscape)");let two=wide.matches,twoSaved=false;
try{const v=localStorage.getItem("trojanTwo");if(v!==null){two=v==="1";twoSaved=true}}catch(_){}
function setTwo(v,save){two=v;$("#drive").classList.toggle("two",v);$("#bTwo").setAttribute("aria-pressed",v);
 $("#hintL").textContent=v?"Move":"Up to walk, sideways to turn";stk.lx=stk.ly=stk.rx=stk.ry=0;
 if(save){twoSaved=true;try{localStorage.setItem("trojanTwo",v?"1":"0")}catch(_){}}}
wide.addEventListener("change",e=>{if(!twoSaved)setTwo(e.matches)});
$("#bTwo").onclick=()=>{setTwo(!two,true);toast(two?"Two joysticks":"One joystick")};
const typing=e=>/INPUT|TEXTAREA|SELECT/.test(e.target.tagName);
addEventListener("keydown",e=>{if(typing(e))return;const k=e.key.toLowerCase();
 if(!e.repeat&&/^[0-9]$/.test(k)){const c=$$("#quick .btn")[(+k+9)%10];if(c)c.click();return}
 if(k==="escape"){estop();return}
 if(k===" "){e.preventDefault();for(const q in keys)keys[q]=0;return}
 if(k.startsWith("arrow"))e.preventDefault();keys[k]=1});
addEventListener("keyup",e=>{keys[e.key.toLowerCase()]=0});
let boostBtn=false;const boostOn=()=>boostBtn||!!keys.shift;
{const b=$("#boost"),on=v=>{boostBtn=v;b.classList.toggle("on",v)};b.addEventListener("pointerdown",e=>{b.setPointerCapture(e.pointerId);on(true)});["pointerup","pointercancel","lostpointercapture"].forEach(t=>b.addEventListener(t,()=>on(false)))}
addEventListener("blur",()=>{for(const k in keys)keys[k]=0});
document.addEventListener("visibilitychange",()=>{if(document.hidden){for(const k in keys)keys[k]=0;stk.lx=stk.ly=stk.rx=stk.ry=0}});
let pad={x:0,y:0,w:0},padBtn=[];
function readPad(){const l=navigator.getGamepads?[...navigator.getGamepads()]:[],g=l.find(Boolean);pad={x:0,y:0,w:0};if(!g)return;
 const dz=v=>Math.abs(v)<.15?0:v;pad.x=dz(g.axes[0]||0);pad.y=-dz(g.axes[1]||0);pad.w=dz(g.axes[2]||0);
 const act=[[0,()=>stand()],[1,()=>sit()],[2,()=>play("wave")],[3,()=>play("groove")],[8,()=>estop()]];
 act.forEach(([i,f])=>{const p=g.buttons[i]&&g.buttons[i].pressed;if(p&&!padBtn[i])f();padBtn[i]=p})}
const K=k=>keys[k]?1:0;
let last="",zeros=0;
setInterval(()=>{readPad();
 // robot frame: +x forward, +y left, +w turn left (counter-clockwise)
 const fwd=stk.ly+K("w")+K("arrowup")-K("s")-K("arrowdown")+pad.y;
 const side=(two?stk.lx+pad.x:0)+K("d")-K("a");
 const turn=(two?-stk.rx-pad.w:-stk.lx-pad.x)+K("q")+K("arrowleft")-K("e")-K("arrowright");
 let vx=clamp(fwd,-1,1),vy=clamp(-side,-1,1),vw=clamp(turn,-1,1);
 if(replay&&Math.abs(vx)+Math.abs(vy)+Math.abs(vw)>.1){replay=null;mInfo()}          // touching the controls cancels a replay
 const rc=replayCmd();if(rc)[vx,vy,vw]=rc;else recordCmd([+vx.toFixed(2),+vy.toFixed(2),+vw.toFixed(2)]);
 const bo=boostOn()?1:0,cur=[vx,vy,vw].map(v=>v.toFixed(2)).join(",")+bo,moving=Math.abs(vx)+Math.abs(vy)+Math.abs(vw)>.05;
 driving=moving;if(moving)zeros=0;
 if((moving||cur!==last||zeros<3)&&!inflight){if(!moving)zeros++;last=cur;inflight=1;api(`/api/c?x=${vx.toFixed(2)}&y=${vy.toFixed(2)}&w=${vw.toFixed(2)}&b=${bo}`).finally(()=>inflight=0)}},70);

/* ---------- commands ---------- */
const stand=()=>{api("/api/a?a=stand");toast("Standing up")},sit=()=>{api("/api/a?a=sit");toast("Sitting down")};
const estop=()=>{replay=null;mInfo();api("/api/a?a=estop");toast("Stopped")};
const play=n=>{api("/api/play?n="+encodeURIComponent(n)).then(r=>r||toast("TROJAN could not play that"))};
$("#estop").onclick=estop;$("#bStand").onclick=stand;$("#bSit").onclick=sit;
$("#bRelax").onclick=()=>{api("/api/a?a=relax");toast("Motors off. Any command turns them back on.")};
$("#bStopAnim").onclick=()=>api("/api/a?a=stand");$("#bShow").onclick=()=>{api("/api/play?show=1");toast("Playing all dances")};
[["wave","Hi"],["paw","Paw"],["nod","Yes"],["shake","No"],["bow","Bow"],["pounce","Pounce"],["wiggle","Wiggle"],["spin","Spin"],["disco","Disco"],["groove","Groove"]].forEach(([n,l])=>{const b=document.createElement("button");b.className="btn";b.textContent=l;b.onclick=()=>play(n);$("#quick").appendChild(b)});
document.addEventListener("click",e=>{if(e.target.closest("button")&&navigator.vibrate)try{navigator.vibrate(8)}catch(_){}});

/* ---------- settings ---------- */
const FMT={tempo:2,freq:2,stride:0,lift:0,height:0,steer:2,maxSpeed:0,deadMs:0,sleepSec:0,relaxSec:0};
const sendCfg=throttle((k,v)=>api(`/api/set?${k}=${v}`),120);
for(const k in FMT){const el=$("#"+k);el.addEventListener("input",()=>{$("#v-"+k).textContent=(+el.value).toFixed(FMT[k]);sendCfg(k,el.value)})}
const SPEEDS=[[0.7,16],[1.0,20],[1.6,26]];
function markSpeed(){const i=SPEEDS.findIndex(([f,s])=>Math.abs(CFG.freq-f)<.03&&Math.abs(CFG.stride-s)<.6);$$("#speedSeg button").forEach((b,j)=>b.classList.toggle("on",j===i))}
function fillCfg(){for(const k in FMT){const el=$("#"+k);if(CFG[k]!==undefined){el.value=CFG[k];$("#v-"+k).textContent=(+CFG[k]).toFixed(FMT[k])}}
 $$("#gaitSeg button").forEach(b=>b.classList.toggle("on",+b.dataset.g===CFG.mode));$("#gaitInfo").textContent=GAITS[CFG.mode]||"";
 $("#bLife").setAttribute("aria-pressed",!!CFG.life);$("#bReact").setAttribute("aria-pressed",!!CFG.react);
 for(let i=0;i<4;i++)["ts","th","tl"].forEach(p=>{const el=$("#"+p+i);el.value=CFG[p][i];$("#v-"+p+i).textContent=(+CFG[p][i]).toFixed(2)});
 if(CFG.size!==undefined){$("#size").value=CFG.size;$("#v-size").textContent=pct(CFG.size)}
 updSv();markSpeed()}
async function loadCfg(){const c=await jget("/api/cfg");if(c){CFG=c;fillCfg()}}
$("#speedSeg").onclick=async e=>{const b=e.target.closest("button");if(!b)return;const[f,st]=SPEEDS[+b.dataset.p];await api(`/api/set?freq=${f}&stride=${st}`);loadCfg()};
$("#gaitSeg").onclick=e=>{const b=e.target.closest("button");if(!b)return;CFG.mode=+b.dataset.g;api("/api/set?mode="+b.dataset.g);fillCfg()};
$$(".save").forEach(b=>b.onclick=async()=>{toast(await api("/api/save")?"Saved on TROJAN":"Not saved, check the connection")});
$("#bLife").onclick=()=>{CFG.life=CFG.life?0:1;api("/api/set?life="+CFG.life);fillCfg()};
$("#bReact").onclick=()=>{CFG.react=CFG.react?0:1;api("/api/set?react="+CFG.react);fillCfg()};
$("#sReboot").onclick=()=>{if(confirm("Restart TROJAN?"))api("/api/sys?a=reboot")};
$("#sReset").onclick=async()=>{if(confirm("Reset every setting to the factory defaults? Your saved dances and faces stay."))if(await api("/api/reset")){loadCfg();toast("Settings reset")}};
$("#wForm").onsubmit=async e=>{e.preventDefault();const p=$("#wPass").value;if(p.length<8||p.length>63||/[^\x20-\x7e]/.test(p))return toast("Use 8 to 63 letters, numbers or symbols");
 if(!confirm("Change the Wi-Fi password? TROJAN restarts, and every device must join again with the new password."))return;
 const r=await api("/api/wifipass",{method:"POST",headers:{"Content-Type":"application/x-www-form-urlencoded"},body:"pass="+encodeURIComponent(p)});
 toast(r?"Password changed. Join Sesame-Controller again with the new one.":"Password not changed")};

/* ---------- play ---------- */
function tiles(sel,list){$(sel).innerHTML="";list.forEach(x=>{const b=document.createElement("button");b.className="btn";b.textContent=x.l;b.onclick=()=>play(x.n);$(sel).appendChild(b)})}
const esc=s=>String(s).replace(/[&<>"]/g,c=>({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;"}[c]));
$("#bSurprise").onclick=()=>{const all=[...MOVES.tricks,...MOVES.builtin];if(!all.length)return;const x=all[Math.floor(Math.random()*all.length)];toast(x.l);play(x.n)};
$("#tempo").addEventListener("change",()=>loadDances());
async function loadDances(){const d=await jget("/api/dances");if(!d)return;MOVES=d;[...d.tricks,...d.drive,...d.builtin].forEach(x=>LABEL[x.n]=x.l);
 tiles("#gestures",d.tricks);tiles("#dtricks",d.drive);
 $("#dances").innerHTML=d.builtin.map(x=>`<div class="item"><div class="n">${esc(x.l)}</div><span class="t">${x.t} s</span><button data-p="${esc(x.n)}">Play</button></div>`).join("");
 $("#udances").innerHTML=d.user.length?d.user.map(n=>`<div class="item"><div class="n">${esc(n)}</div><button data-p="u:${esc(n)}">Play</button><button data-e="${esc(n)}">Edit</button><button data-f="${esc(n)}">Save file</button><button data-d="${esc(n)}">Delete</button></div>`).join(""):'<p class="note" style="margin:0">None yet. Make one in More, Pose lab and dance builder.</p>'}
$("#t-play").addEventListener("click",async e=>{const b=e.target.closest(".item button");if(!b)return;
 if(b.dataset.p)play(b.dataset.p);
 if(b.dataset.d&&confirm("Delete "+b.dataset.d+"?")){await api("/api/ddel?n="+encodeURIComponent(b.dataset.d));loadDances()}
 if(b.dataset.e){await editDance(b.dataset.e);openSection("#dLab")}
 if(b.dataset.f){const r=await api("/api/dget?n="+encodeURIComponent(b.dataset.f));if(!r)return toast("Could not read that dance");
  const a=document.createElement("a");a.href=URL.createObjectURL(new Blob([await r.text()],{type:"text/plain"}));a.download=b.dataset.f+".txt";a.click();setTimeout(()=>URL.revokeObjectURL(a.href),1000)}});
$("#dImp").onclick=()=>$("#dFile").click();
$("#dFile").onchange=async e=>{const f=e.target.files[0];e.target.value="";if(!f)return;
 const n=f.name.replace(/\.[^.]*$/,"").replace(/[^A-Za-z0-9_-]/g,"").slice(0,16)||"dance";
 const r=await api("/api/dsave?n="+encodeURIComponent(n),{method:"POST",headers:{"Content-Type":"text/plain"},body:await f.text()});
 toast(r?"Added "+n:"That file is not a TROJAN dance");loadDances()};
const pct=v=>Math.round(v*100)+"%";
$("#size").addEventListener("input",e=>{$("#v-size").textContent=pct(e.target.value);sendCfg("size",e.target.value)});

/* ---------- face ---------- */
MOODS.forEach(([i,n])=>{const b=document.createElement("button");b.className="btn";b.textContent=n;b.onclick=()=>api("/api/mood?m="+i);$("#moods").appendChild(b)});
SCREENS.forEach(([i,n])=>{const b=document.createElement("button");b.className="btn";b.textContent=n;b.onclick=()=>api("/api/screen?id="+i);$("#screens").appendChild(b)});
$("#bBanner").onclick=()=>{const t=$("#bannerTxt").value.trim();if(t)api("/api/text?t="+encodeURIComponent(t)+"&ms=5000")};
const fc=$("#fcv"),fx=fc.getContext("2d",{willReadFrequently:true});let tool=1,mirror=false,drawing=false,lp=null;
function fclear(v){fx.fillStyle=v?"#fff":"#000";fx.fillRect(0,0,128,64)}fclear(0);
function fpt(e){const b=fc.getBoundingClientRect();return[Math.floor((e.clientX-b.left)/b.width*128),Math.floor((e.clientY-b.top)/b.height*64)]}
function dab(x,y){const s=+$("#brush").value;fx.fillStyle=tool?"#fff":"#000";const o=Math.floor(s/2);fx.fillRect(x-o,y-o,s,s);if(mirror)fx.fillRect(127-x-o,y-o,s,s)}
function line(a,b){let[x0,y0]=a,[x1,y1]=b,dx=Math.abs(x1-x0),dy=-Math.abs(y1-y0),sx=x0<x1?1:-1,sy=y0<y1?1:-1,er=dx+dy;for(;;){dab(x0,y0);if(x0===x1&&y0===y1)break;const e2=2*er;if(e2>=dy){er+=dy;x0+=sx}if(e2<=dx){er+=dx;y0+=sy}}}
fc.addEventListener("pointerdown",e=>{drawing=true;fc.setPointerCapture(e.pointerId);lp=fpt(e);dab(...lp)});
fc.addEventListener("pointermove",e=>{if(!drawing)return;const p=fpt(e);line(lp,p);lp=p});
fc.addEventListener("pointerup",()=>drawing=false);fc.addEventListener("pointercancel",()=>drawing=false);
$("#toolSeg").onclick=e=>{const b=e.target.closest("button");if(!b)return;tool=+b.dataset.t;$$("#toolSeg button").forEach(x=>x.classList.toggle("on",x===b))};
$("#fMirror").onclick=e=>{mirror=!mirror;e.currentTarget.setAttribute("aria-pressed",mirror)};
$("#fClear").onclick=()=>fclear(0);
$("#fInvert").onclick=()=>{const d=fx.getImageData(0,0,128,64);for(let i=0;i<d.data.length;i+=4){const v=d.data[i]>127?0:255;d.data[i]=d.data[i+1]=d.data[i+2]=v}fx.putImageData(d,0,0)};
$("#fFlip").onclick=()=>{const c=document.createElement("canvas");c.width=128;c.height=64;c.getContext("2d").drawImage(fc,0,0);fx.save();fx.translate(128,0);fx.scale(-1,1);fx.drawImage(c,0,0);fx.restore()};
$("#thr").oninput=e=>$("#v-thr").textContent=e.target.value;
$("#fImg").onclick=()=>$("#fFile").click();
$("#fFile").onchange=e=>{const f=e.target.files[0];if(!f)return;const im=new Image();im.onload=()=>{const c=document.createElement("canvas");c.width=128;c.height=64;const g=c.getContext("2d");g.fillStyle="#fff";g.fillRect(0,0,128,64);const r=Math.min(128/im.width,64/im.height);g.drawImage(im,(128-im.width*r)/2,(64-im.height*r)/2,im.width*r,im.height*r);
 const d=g.getImageData(0,0,128,64),t=+$("#thr").value;for(let i=0;i<d.data.length;i+=4){const l=.3*d.data[i]+.59*d.data[i+1]+.11*d.data[i+2];const v=l<t?255:0;d.data[i]=d.data[i+1]=d.data[i+2]=v}fx.putImageData(d,0,0);URL.revokeObjectURL(im.src)};im.src=URL.createObjectURL(f);e.target.value=""};
$("#fTxt").onclick=()=>{const t=prompt("Text to add (about 10 characters fit)");if(!t)return;fx.fillStyle="#fff";fx.font="bold 24px ui-monospace,Menlo,monospace";fx.textAlign="center";fx.textBaseline="middle";fx.fillText(t,64,32,124);
 const d=fx.getImageData(0,0,128,64);for(let i=0;i<d.data.length;i+=4){const v=d.data[i]>110?255:0;d.data[i]=d.data[i+1]=d.data[i+2]=v}fx.putImageData(d,0,0)};
function canvasHex(){const d=fx.getImageData(0,0,128,64).data;let h="";for(let y=0;y<64;y++)for(let bx=0;bx<16;bx++){let v=0;for(let b=0;b<8;b++){v=(v<<1)|(d[((y*128)+bx*8+b)*4]>127?1:0)}h+=v.toString(16).padStart(2,"0")}return h}
function hexToCanvas(h){const im=fx.createImageData(128,64);for(let y=0;y<64;y++)for(let bx=0;bx<16;bx++){const v=parseInt(h.substr((y*16+bx)*2,2),16);for(let b=0;b<8;b++){const o=((y*128)+bx*8+b)*4,on=(v>>(7-b))&1?255:0;im.data[o]=im.data[o+1]=im.data[o+2]=on;im.data[o+3]=255}}fx.putImageData(im,0,0)}
const slotName=()=>"slot "+(+$("#slot").value+1);
$("#fPrev").onclick=()=>api("/api/fshow",{method:"POST",headers:{"Content-Type":"text/plain"},body:canvasHex()});
$("#fSave").onclick=async()=>{const r=await api("/api/fsave?slot="+$("#slot").value,{method:"POST",headers:{"Content-Type":"text/plain"},body:canvasHex()});toast(r?"Saved to "+slotName():"Not saved");slotInfo()};
$("#fLoad").onclick=async()=>{const r=await api("/api/fget?slot="+$("#slot").value);if(r)hexToCanvas((await r.text()).trim());else toast(slotName()+" is empty")};
$("#fShow").onclick=()=>api("/api/screen?id="+(64+ +$("#slot").value));
$("#fDel").onclick=async()=>{if(confirm("Delete "+slotName()+"?")){await api("/api/fdel?slot="+$("#slot").value);slotInfo()}};
async function slotInfo(){const l=await jget("/api/flist");if(l)$("#slotInfo").textContent=l.length?"Saved: "+l.map(i=>"slot "+(i+1)).join(", "):"No saved faces yet."}

/* ---------- pose lab and dance builder ---------- */
const lab={s:[0,0,0,0],a:[0,0,0,0]};let linkLR=false,liveOn=true,kfs=[];
function mkSliders(){let h="",f="";LEGS.forEach((n,i)=>{h+=`<label class="f" for="ls${i}">${n} <output id="v-ls${i}">0</output></label><input type="range" id="ls${i}" min="-45" max="45" value="0">`;f+=`<label class="f" for="la${i}">${n} <output id="v-la${i}">0</output></label><input type="range" id="la${i}" min="0" max="120" value="0">`});$("#hipSl").innerHTML=h;$("#footSl").innerHTML=f;
 for(let i=0;i<4;i++)["s","a"].forEach(k=>$("#l"+k+i).addEventListener("input",e=>{lab[k][i]=+e.target.value;if(linkLR)lab[k][[2,3,0,1][i]]=lab[k][i];syncLab();sendLab()}))}
function syncLab(){for(let i=0;i<4;i++)["s","a"].forEach(k=>{$("#l"+k+i).value=lab[k][i];$("#v-l"+k+i).textContent=lab[k][i]})}
const sendLabNow=throttle(()=>{if(liveOn)api("/api/pose?"+lab.s.map((v,i)=>"s"+i+"="+v).join("&")+"&"+lab.a.map((v,i)=>"a"+i+"="+v).join("&"))},90);
// The keep-alive only re-sends a pose the user set; a dance preview or Stop releases it (a resend would cancel the preview).
let labHold=false;const sendLab=()=>{labHold=true;sendLabNow()};
setInterval(()=>{if(curTab()==="more"&&$("#dLab").open&&liveOn&&labHold)sendLabNow()},1100);
mkSliders();
$("#lZero").onclick=()=>{lab.s=[0,0,0,0];lab.a=[0,0,0,0];syncLab();sendLab()};
$("#lCopy").onclick=()=>{if(!ST.s)return;lab.s=ST.s.map(Math.round);lab.a=ST.a.map(Math.round);syncLab();toast("Copied. Add it as a step or change it.")};
$("#lMirror").onclick=e=>{linkLR=!linkLR;e.currentTarget.setAttribute("aria-pressed",linkLR)};
$("#lLive").onclick=e=>{liveOn=!liveOn;e.currentTarget.setAttribute("aria-pressed",liveOn)};
const PRE={"Stand":[[0,0,0,0],[0,0,0,0]],"Sit":[[0,0,0,0],[75,75,75,75]],"Crouch":[[0,0,0,0],[30,30,30,30]],"Bow":[[0,0,0,0],[48,0,48,0]],"Rear up":[[0,0,0,0],[0,45,0,45]],"Twist":[[26,26,-26,-26],[0,0,0,0]],"Legs forward":[[22,22,22,22],[0,0,0,0]],"High five":[[28,0,0,0],[70,0,0,0]]};
Object.keys(PRE).forEach(k=>{const o=document.createElement("option");o.textContent=k;$("#lPre").appendChild(o)});
$("#lPre").onchange=e=>{const p=PRE[e.target.value];if(!p)return;lab.s=[...p[0]];lab.a=[...p[1]];syncLab();sendLab();e.target.value=""};
[[0,"Face: keep as is"],...MOODS,...SCREENS].forEach(([i,n])=>{const o=document.createElement("option");o.value=i;o.textContent=n;$("#kFace").appendChild(o)});
function renderK(){let t=0;$("#kRows").innerHTML=kfs.map((k,i)=>{t+=k.ms;return`<tr><td>${i+1}</td><td>${k.ms}</td><td>${k.s.join(" ")}</td><td>${k.a.join(" ")}</td><td><button data-g="${i}">Go</button><button data-u="${i}">Up</button><button data-x="${i}">Remove</button></td></tr>`}).join("");
 $("#kTot").textContent=kfs.length?kfs.length+" steps, "+(t/1000).toFixed(1)+" s in total":"No steps yet."}
$("#kAdd").onclick=()=>{kfs.push({ms:+$("#kMs").value||500,s:[...lab.s],a:[...lab.a],f:+$("#kFace").value});renderK()};
$("#kRows").onclick=e=>{const b=e.target.closest("button");if(!b)return;if(b.dataset.x!==undefined)kfs.splice(+b.dataset.x,1);if(b.dataset.u!==undefined){const i=+b.dataset.u;if(i>0)[kfs[i-1],kfs[i]]=[kfs[i],kfs[i-1]]}if(b.dataset.g!==undefined){const k=kfs[+b.dataset.g];lab.s=[...k.s];lab.a=[...k.a];syncLab();sendLab()}renderK()};
$("#kClear").onclick=()=>{if(!kfs.length||confirm("Clear all steps?")){kfs=[];renderK()}};
const kText=()=>kfs.map(k=>[k.ms,...k.s,...k.a,k.f].join(" ")).join("\n");
$("#kPrev").onclick=async()=>{if(!kfs.length)return toast("Add a step first");labHold=false;const r=await api("/api/dplay",{method:"POST",headers:{"Content-Type":"text/plain"},body:kText()});if(!r)toast("TROJAN could not play that dance")};
$("#kStop").onclick=()=>{labHold=false;api("/api/a?a=stand")};
$("#kSave").onclick=async()=>{const n=$("#kName").value.trim();if(!n||!kfs.length)return toast("Give the dance a name and at least one step");const r=await api("/api/dsave?n="+encodeURIComponent(n),{method:"POST",headers:{"Content-Type":"text/plain"},body:kText()});toast(r?"Saved "+n:"Not saved");loadDances()};
async function editDance(n){const r=await api("/api/dget?n="+encodeURIComponent(n));if(!r)return;kfs=(await r.text()).split("\n").map(l=>l.trim()).filter(l=>l&&l[0]!=="#").map(l=>{const v=l.split(/\s+/).map(Number);return{ms:v[0],s:v.slice(1,5),a:v.slice(5,9),f:v[9]||0}});$("#kName").value=n;renderK()}
/* ---------- live leg view (top-down, robot faces right). Eased toward each new state so it moves smoothly
   between polls. Orange = foot on the ground, cyan = foot lifted. ---------- */
let view="legs";try{view=localStorage.getItem("trojanView")||"legs"}catch(_){}
function setView(v){view=v;$("#scr").classList.toggle("face",v==="face");$$(".vsw button").forEach(b=>b.classList.toggle("on",b.dataset.v===v));try{localStorage.setItem("trojanView",v)}catch(_){}}
$(".vsw").onclick=e=>{const b=e.target.closest("button");if(b)setView(b.dataset.v)};
const disp={s:[0,0,0,0],a:[0,0,0,0]},LEGK=["FR","RR","FL","RL"];
function drawLegs(c){const g=c.getContext("2d"),W=c.width,H=c.height,cx=W/2,cy=H/2;
 g.fillStyle="#0C1118";g.fillRect(0,0,W,H);g.strokeStyle="#1A2431";g.lineWidth=1;g.beginPath();
 for(let x=20;x<W;x+=40){g.moveTo(x,0);g.lineTo(x,H)}for(let y=20;y<H;y+=40){g.moveTo(0,y);g.lineTo(W,y)}g.stroke();
 const hips=[[78,53],[-78,53],[78,-53],[-78,-53]],L=106;                      // FR RR FL RL: [forward, right]
 g.lineCap="round";g.font="600 20px system-ui";
 for(let i=0;i<4;i++){const s=disp.s[i],th=(i===0?45-s:i===1?135-s:i===2?-45+s:-135+s)*Math.PI/180,hx=cx+hips[i][0],hy=cy+hips[i][1],
  fx2=hx+Math.cos(th)*L*1.1,fy=hy+Math.sin(th)*L*.85,up=clamp(disp.a[i]/45,0,1),air=up>.15;
  g.strokeStyle=air?"#35D6FF":"#FF8A3D";g.lineWidth=10;g.beginPath();g.moveTo(hx,hy);g.lineTo(fx2,fy);g.stroke();
  g.fillStyle=air?"#35D6FF":"#FFB27A";g.beginPath();g.arc(fx2,fy,9+up*10,0,7);g.fill();
  g.fillStyle="#8593A8";g.fillText(LEGK[i],fx2+(fx2>cx?16:-40),fy+6)}
 g.fillStyle="#232F43";g.strokeStyle="#35D6FF";g.lineWidth=3;g.beginPath();g.roundRect?g.roundRect(cx-120,cy-46,240,92,18):g.rect(cx-120,cy-46,240,92);g.fill();g.stroke();
 g.fillStyle="#35D6FF";g.beginPath();g.arc(cx+95,cy-15,8,0,7);g.arc(cx+95,cy+15,8,0,7);g.fill()}
function drawSim(){const s=ST.s||[0,0,0,0],a=ST.a||[0,0,0,0];for(let i=0;i<4;i++){disp.s[i]+=(s[i]-disp.s[i])*.25;disp.a[i]+=(a[i]-disp.a[i])*.25}
 const t=curTab();if(t==="drive"||t==="more")$$(".legsview").forEach(c=>{if(c.offsetParent)drawLegs(c)});requestAnimationFrame(drawSim)}
drawSim();

/* ---------- leg tuning and servo test ---------- */
$("#legTune").innerHTML=LEGS.map((n,i)=>`<div><h3>${n}</h3><label class="f" for="ts${i}">Step length <output id="v-ts${i}"></output></label><input type="range" id="ts${i}" min="0.5" max="1.5" step="0.01"><label class="f" for="th${i}">Hip offset <output id="v-th${i}"></output></label><input type="range" id="th${i}" min="-15" max="15" step="0.5"><label class="f" for="tl${i}">Foot lift <output id="v-tl${i}"></output></label><input type="range" id="tl${i}" min="0.5" max="1.6" step="0.01"></div>`).join("");
for(let i=0;i<4;i++)["ts","th","tl"].forEach(p=>{const el=$("#"+p+i);el.addEventListener("input",()=>{$("#v-"+p+i).textContent=(+el.value).toFixed(2);sendCfg(p+i,el.value)})});
$("#tNeutral").onclick=async()=>{if(!confirm("Reset the tuning of all four legs and all motor offsets?"))return;for(let i=0;i<4;i++)await api(`/api/set?ts${i}=1&th${i}=0&tl${i}=1`);for(let i=0;i<8;i++)await api(`/api/set?st${i}=0`);loadCfg()};
SERVOS.forEach((n,i)=>{const o=document.createElement("option");o.value=i;o.textContent="Motor "+n;$("#sv").appendChild(o)});
function updSv(){const i=+$("#sv").value,r=$("#svr");r.min=SMIN[i];r.max=SMAX[i];r.value=clamp(+r.value,SMIN[i],SMAX[i]);$("#v-sv").textContent=r.value;if(CFG.st){$("#svt").value=CFG.st[i];$("#v-svt").textContent=CFG.st[i]}}
const sendSv=throttle(()=>api(`/api/raw?i=${$("#sv").value}&v=${$("#svr").value}`),90);
let svActive=false;setInterval(()=>{if(curTab()==="more"&&$("#dServo").open&&svActive)sendSv()},1500);
const svSet=v=>{$("#svr").value=v;svActive=true;$("#v-sv").textContent=$("#svr").value;sendSv()};
$("#sv").onchange=()=>{svActive=false;updSv()};
$("#svr").oninput=()=>svSet($("#svr").value);
$("#svM").onclick=()=>svSet(+$("#svr").value-1);$("#svP").onclick=()=>svSet(+$("#svr").value+1);
$("#svC").onclick=()=>{const i=+$("#sv").value;svSet((SMIN[i]+SMAX[i])/2)};
$("#svt").oninput=()=>{const i=+$("#sv").value;$("#v-svt").textContent=$("#svt").value;if(CFG.st)CFG.st[i]=+$("#svt").value;sendCfg("st"+i,$("#svt").value)};

/* ---------- all motors at once (raw angles; each held 6 s by the robot, re-sent while touched) ---------- */
const SDESC=["front right hip","back right hip","front left hip","back left hip","back right foot","front right foot","front left foot","back left foot"];
$("#allSv").innerHTML=SERVOS.map((n,i)=>`<div><label class="f" for="am${i}">${n}, ${SDESC[i]} <output id="v-am${i}"></output></label><input type="range" id="am${i}" min="${SMIN[i]}" max="${SMAX[i]}"></div>`).join("");
const amHeld=new Set(),amSend=SERVOS.map((_,i)=>throttle(()=>api(`/api/raw?i=${i}&v=${$("#am"+i).value}`),90));
function amFill(){if(!ST.raw)return;SERVOS.forEach((_,i)=>{$("#am"+i).value=Math.round(ST.raw[i]);$("#v-am"+i).textContent=$("#am"+i).value})}
SERVOS.forEach((_,i)=>$("#am"+i).addEventListener("input",e=>{$("#v-am"+i).textContent=e.target.value;amHeld.add(i);amSend[i]()}));
$("#dAll").addEventListener("toggle",()=>{if($("#dAll").open)amFill();else amHeld.clear()});
$("#amFill").onclick=amFill;
$("#amFree").onclick=()=>{amHeld.clear();toast("Motors go back to normal in a few seconds")};
setInterval(()=>{if(curTab()==="more"&&$("#dAll").open)amHeld.forEach(i=>amSend[i]())},1500);

/* ---------- route recorder ---------- */
let rec=null,recT0=0,path=[],replay=null;
try{path=JSON.parse(localStorage.getItem("trojanPath")||"[]")}catch(_){path=[]}
function mInfo(){$("#mInfo").textContent=rec?"Recording. Drive now.":replay?"Replaying the route.":path.length?"Saved route: "+(path[path.length-1][0]/1000).toFixed(1)+" s":"No route recorded yet.";
 $("#mRec").textContent=rec?"Stop recording":"Record";$("#mPlay").textContent=replay?"Stop replay":"Replay"}
$("#mRec").onclick=()=>{if(rec){path=rec;rec=null;try{localStorage.setItem("trojanPath",JSON.stringify(path))}catch(_){}toast("Route saved")}else{replay=null;rec=[];recT0=Date.now()}mInfo()};
$("#mPlay").onclick=()=>{if(replay){replay=null;mInfo();return}if(!path.length)return toast("Record a route first");rec=null;replay={t0:Date.now(),i:0};mInfo()};
// [vx,vy,vw] from the replay, or null; the send loop calls it every 70 ms
function replayCmd(){if(!replay)return null;const t=Date.now()-replay.t0;while(replay.i<path.length-1&&path[replay.i+1][0]<=t)replay.i++;if(t>path[path.length-1][0]+300){replay=null;mInfo();return null}return path[replay.i].slice(1)}
function recordCmd(v){if(!rec)return;const t=Date.now()-recT0,l=rec[rec.length-1];if(!l||l[1]!==v[0]||l[2]!==v[1]||l[3]!==v[2])rec.push([t,...v]);if(t>120000)$("#mRec").click()}

/* ---------- live face (paused while driving so drive commands get the link) ---------- */
const mimg=new ImageData(128,64);let mBusy=0;
async function pollMirror(){const t=curTab();if((t==="face"||(t==="drive"&&view==="face"))&&!mBusy&&online&&!driving){mBusy=1;const r=await api("/api/fb");if(r){const h=(await r.text()).trim();
 for(let i=0;i<1024;i++){const v=parseInt(h.substr(i*2,2),16),x=i%128,pg=i>>7;for(let b=0;b<8;b++){const o=((pg*8+b)*128+x)*4,on=(v>>b)&1;mimg.data[o]=on?232:0;mimg.data[o+1]=on?244:0;mimg.data[o+2]=on?255:0;mimg.data[o+3]=255}}
 $$(".mirror").forEach(c=>c.getContext("2d").putImageData(mimg,0,0))}mBusy=0}setTimeout(pollMirror,250)}

setTwo(two);setView(view);mInfo();renderK();loadCfg();poll();loadDances();pollMirror();
</script></body></html>)TROJANHTML";
