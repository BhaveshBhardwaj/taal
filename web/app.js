// TAAL Visualizer Engine
(function() {
  const canvas = document.getElementById('kuramotoCanvas');
  const ctx = canvas.getContext('2d');

  let currentMode = 'unison'; // 'unison' or 'polyrhythm'

  // Oscillator particles
  const NUM_PARTICLES = 160;
  let particles = [];

  function initParticles() {
    particles = [];
    for (let i = 0; i < NUM_PARTICLES; ++i) {
      // Unison: tightly clustered around 21:00 (phase ~ 5.5 rad)
      const basePhase = 5.5; // ~21:00 on 24-hr clock
      const unisonPhase = basePhase + (Math.random() - 0.5) * 0.45;

      // Polyrhythm: 4 equal waves at 0, pi/2, pi, 3pi/2 (0m, 12m, 24m, 36m)
      const wave = i % 4;
      const polyPhase = (wave * (Math.PI / 2)) + (Math.random() - 0.5) * 0.25;

      particles.push({
        unisonPhase: unisonPhase,
        polyPhase: polyPhase,
        currentPhase: unisonPhase,
        wave: wave,
        radius: 125 + (Math.random() - 0.5) * 20,
        speed: 0.008 + Math.random() * 0.004
      });
    }
  }

  initParticles();

  // Mode switching
  const btnUnison = document.getElementById('btnUnison');
  const btnPolyrhythm = document.getElementById('btnPolyrhythm');

  const valDpi = document.getElementById('valDpi');
  const capDpi = document.getElementById('capDpi');
  const barDpi = document.getElementById('barDpi');

  const valGate = document.getElementById('valGate');
  const capGate = document.getElementById('capGate');
  const barGate = document.getElementById('barGate');

  const valFeeder = document.getElementById('valFeeder');
  const capFeeder = document.getElementById('capFeeder');
  const barFeeder = document.getElementById('barFeeder');

  const valCapacity = document.getElementById('valCapacity');
  const capCapacity = document.getElementById('capCapacity');
  const barCapacity = document.getElementById('barCapacity');

  const mapGate = document.getElementById('mapGate');
  const mapMetro = document.getElementById('mapMetro');
  const mapBattery = document.getElementById('mapBattery');

  function updateMode(mode) {
    currentMode = mode;
    if (mode === 'unison') {
      btnUnison.classList.add('active');
      btnPolyrhythm.classList.remove('active');

      valDpi.textContent = '0.80';
      capDpi.textContent = 'High Coherence (Lockstep Unison)';
      barDpi.style.width = '80%';
      barDpi.className = 'bar-fill alert';

      valGate.textContent = '8.4 min';
      capGate.textContent = 'Lahore Gate Chokepoint';
      barGate.style.width = '70%';
      barGate.className = 'bar-fill alert';

      valFeeder.textContent = '100.1%';
      capFeeder.textContent = 'Transformer Overloaded (1,001 kVA)';
      barFeeder.style.width = '100%';
      barFeeder.className = 'bar-fill alert';

      valCapacity.textContent = 'Baseline';
      capCapacity.textContent = 'Stress Threshold Breached';
      barCapacity.style.width = '55%';
      barCapacity.className = 'bar-fill warning';

      mapGate.textContent = 'Wait: 8.4m';
      mapMetro.textContent = 'Platform: 0.48 p/m²';
      mapBattery.textContent = 'BESS Buffer: Standby';
    } else {
      btnPolyrhythm.classList.add('active');
      btnUnison.classList.remove('active');

      valDpi.textContent = '0.01';
      capDpi.textContent = 'Phase De-correlated (Polyrhythm)';
      barDpi.style.width = '5%';
      barDpi.className = 'bar-fill success';

      valGate.textContent = '0.0 min';
      capGate.textContent = 'Rolling Waves (Zero Wait)';
      barGate.style.width = '5%';
      barGate.className = 'bar-fill success';

      valFeeder.textContent = '50.5%';
      capFeeder.textContent = 'Safe Grid Loading (505 kVA)';
      barFeeder.style.width = '50%';
      barFeeder.className = 'bar-fill success';

      valCapacity.textContent = '+68.4%';
      capCapacity.textContent = 'Throughput Multiplied!';
      barCapacity.style.width = '95%';
      barCapacity.className = 'bar-fill success';

      mapGate.textContent = 'Wait: 0.0m (4 Waves)';
      mapMetro.textContent = 'Platform: 0.29 p/m² (Safe)';
      mapBattery.textContent = 'BESS Buffer: Active (300kW)';
    }
  }

  btnUnison.addEventListener('click', () => updateMode('unison'));
  btnPolyrhythm.addEventListener('click', () => updateMode('polyrhythm'));

  // Animation Loop
  function draw() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    const cx = canvas.width / 2;
    const cy = canvas.height / 2;
    const R_MAIN = 135;

    // Outer Clock Circle
    ctx.beginPath();
    ctx.arc(cx, cy, R_MAIN, 0, Math.PI * 2);
    ctx.strokeStyle = '#24324a';
    ctx.lineWidth = 3;
    ctx.stroke();

    // 24-Hour Markers
    ctx.fillStyle = '#64748b';
    ctx.font = '11px JetBrains Mono';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';

    for (let h = 0; h < 24; h += 4) {
      const angle = (h / 24) * Math.PI * 2 - Math.PI / 2;
      const x = cx + (R_MAIN + 20) * Math.cos(angle);
      const y = cy + (R_MAIN + 20) * Math.sin(angle);
      ctx.fillText(`${h}:00`, x, y);
    }

    // Compute center-of-mass Kuramoto Order Vector: R * exp(i * Psi)
    let sumCos = 0;
    let sumSin = 0;

    particles.forEach(p => {
      // Lerp phase towards target mode
      const targetPhase = (currentMode === 'unison') ? p.unisonPhase : p.polyPhase;
      p.currentPhase += (targetPhase - p.currentPhase) * 0.08;

      sumCos += Math.cos(p.currentPhase);
      sumSin += Math.sin(p.currentPhase);

      // Particle position
      const px = cx + p.radius * Math.cos(p.currentPhase - Math.PI / 2);
      const py = cy + p.radius * Math.sin(p.currentPhase - Math.PI / 2);

      // Color based on mode / wave
      ctx.beginPath();
      ctx.arc(px, py, 4.5, 0, Math.PI * 2);
      if (currentMode === 'unison') {
        ctx.fillStyle = '#ef4444';
      } else {
        const colors = ['#ef4444', '#10b981', '#38bdf8', '#f59e0b'];
        ctx.fillStyle = colors[p.wave];
      }
      ctx.fill();
    });

    // Kuramoto Order Vector R
    const rParam = Math.sqrt(sumCos * sumCos + sumSin * sumSin) / NUM_PARTICLES;
    const psi = Math.atan2(sumSin, sumCos);

    const arrowLen = Math.max(10, rParam * R_MAIN);
    const arrowX = cx + arrowLen * Math.cos(psi - Math.PI / 2);
    const arrowY = cy + arrowLen * Math.sin(psi - Math.PI / 2);

    // Vector line
    ctx.beginPath();
    ctx.moveTo(cx, cy);
    ctx.lineTo(arrowX, arrowY);
    ctx.strokeStyle = (currentMode === 'unison') ? '#ef4444' : '#10b981';
    ctx.lineWidth = 4;
    ctx.stroke();

    // Center pivot
    ctx.beginPath();
    ctx.arc(cx, cy, 6, 0, Math.PI * 2);
    ctx.fillStyle = '#fff';
    ctx.fill();

    // Text display for R(t)
    ctx.fillStyle = '#f8fafc';
    ctx.font = 'bold 14px JetBrains Mono';
    ctx.fillText(`R(t) = ${rParam.toFixed(2)}`, cx, cy + R_MAIN + 40);

    requestAnimationFrame(draw);
  }

  draw();
})();

