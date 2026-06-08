// Mermaid badge + lightbox.
//
// THE TRICK: mkdocs-material puts the rendered Mermaid SVG inside a
// CLOSED shadow root attached to a <div class="mermaid">. From regular
// JS we cannot enumerate the shadow's contents — `element.shadowRoot`
// returns null, `querySelector('svg')` returns null. The diagram is only
// visible to the user because the browser renders shadow DOM natively.
//
// Implications:
//   * We cannot clone the SVG.
//   * We CAN move the .mermaid div around — the shadow comes with it.
//   * We CAN apply CSS transforms to the div — they cascade visually.
//
// Strategy:
//   1. Watch for <div class="mermaid"> elements that have non-zero size
//      (= shadow root populated = render finished).
//   2. Attach the "Enlarge" badge as a sibling absolute element so it
//      doesn't pollute the .mermaid div's contents (which Material may
//      manage).
//   3. On click, MOVE the .mermaid div into the lightbox stage and leave
//      a placeholder behind so we can restore it on close.
//   4. Wheel/drag apply CSS transforms to the moved div.

(function () {
  var LOG = "[rb-mermaid]";
  var BADGE_CLS = "rb-zoom-badge";
  var PLACEHOLDER_CLS = "rb-mermaid-placeholder";
  var WRAP_CLS = "rb-mermaid-wrap";

  // ---------- Lightbox (one global instance) ----------
  var currentMoved = null;   // the mermaid div currently shown in the lightbox
  var currentPlaceholder = null;  // its placeholder back in the article

  function buildLightbox() {
    if (document.getElementById("rb-mermaid-lightbox")) return;
    var overlay = document.createElement("div");
    overlay.id = "rb-mermaid-lightbox";
    overlay.innerHTML =
      '<button class="rb-lb-close" type="button" aria-label="Close (Esc)">&times;</button>' +
      '<div class="rb-lb-stage"></div>' +
      '<div class="rb-lb-hint">Scroll to zoom · drag to pan · Esc to close</div>';
    document.body.appendChild(overlay);

    var stage = overlay.querySelector(".rb-lb-stage");
    var scale = 1, tx = 0, ty = 0;
    var dragging = false, lastX = 0, lastY = 0;

    // RAF-throttle transform writes. We mutate scale/tx/ty freely but
    // only flush to the DOM once per animation frame.
    var transformPending = false;
    function applyTransform() {
      if (transformPending) return;
      transformPending = true;
      requestAnimationFrame(function () {
        transformPending = false;
        if (currentMoved) {
          currentMoved.style.transform =
            "translate3d(" + tx + "px," + ty + "px,0) scale(" + scale + ")";
        }
      });
    }

    // Crisp-vs-fast trade-off:
    //   * With `will-change: transform` the browser pre-rasterizes the
    //     element at its natural size and lets the GPU composite scale
    //     it. Fast but blurry at high scale (bilinear filtering).
    //   * Without it, the browser re-rasterizes the SVG on every transform
    //     change. Crisp (SVG is vector) but slow on rapid input.
    // We get both by toggling: enable it while the user is interacting
    // (wheel/drag) and remove it after a short idle so the browser
    // re-rasterizes the diagram at the final scale — crisp result.
    var crispTimer = null;
    function bumpInteractive() {
      if (!currentMoved) return;
      currentMoved.style.willChange = "transform";
      clearTimeout(crispTimer);
      crispTimer = setTimeout(function () {
        if (currentMoved) currentMoved.style.willChange = "";
      }, 180);
    }
    function close() {
      overlay.classList.remove("rb-lb-open");
      // Move the diagram back and clear ALL inline styles we applied
      // (including will-change, so the browser releases the GPU layer).
      if (currentMoved && currentPlaceholder) {
        currentMoved.style.transform = "";
        currentMoved.style.maxWidth = "";
        currentMoved.style.maxHeight = "";
        currentMoved.style.cursor = "";
        currentMoved.style.willChange = "";
        currentMoved.style.transformOrigin = "";
        currentMoved.style.display = "";
        currentPlaceholder.parentNode.replaceChild(currentMoved, currentPlaceholder);
      }
      currentMoved = null;
      currentPlaceholder = null;
      document.body.style.overflow = "";
      scale = 1; tx = 0; ty = 0;
    }

    overlay.addEventListener("click", function (e) {
      // Click on overlay background (not on the diagram or close btn) closes.
      if (e.target === overlay || e.target.classList.contains("rb-lb-close")) close();
    });
    document.addEventListener("keydown", function (e) {
      if (e.key === "Escape" && overlay.classList.contains("rb-lb-open")) close();
    });
    overlay.addEventListener("wheel", function (e) {
      if (!overlay.classList.contains("rb-lb-open")) return;
      e.preventDefault();
      // Exponential zoom: proportional to deltaY magnitude.  Works
      // naturally for both wheel mice (deltaY ≈ ±100) and trackpads
      // (smaller, continuous deltaY).  Scroll UP shrinks deltaY → e^pos
      // → zoom IN. Scroll DOWN → zoom OUT.
      var factor = Math.exp(-e.deltaY * 0.0025);
      scale = Math.min(20, Math.max(0.1, scale * factor));
      bumpInteractive();
      applyTransform();
    }, { passive: false });
    overlay.addEventListener("mousedown", function (e) {
      if (!overlay.classList.contains("rb-lb-open")) return;
      if (e.target.closest(".rb-lb-close")) return;
      dragging = true;
      lastX = e.clientX; lastY = e.clientY;
      stage.style.cursor = "grabbing";
      bumpInteractive();
      e.preventDefault();
    });
    document.addEventListener("mousemove", function (e) {
      if (!dragging) return;
      tx += e.clientX - lastX;
      ty += e.clientY - lastY;
      lastX = e.clientX; lastY = e.clientY;
      bumpInteractive();
      applyTransform();
    });
    document.addEventListener("mouseup", function () {
      dragging = false;
      stage.style.cursor = "grab";
    });

    overlay.__reset = function (initial) {
      scale = (typeof initial === "number" && isFinite(initial)) ? initial : 1;
      tx = 0; ty = 0;
      applyTransform();
    };
  }

  function openLightboxWith(mermaidDiv) {
    if (currentMoved) return; // already open
    buildLightbox();
    var overlay = document.getElementById("rb-mermaid-lightbox");
    var stage = overlay.querySelector(".rb-lb-stage");

    // Capture the diagram's article-size and create a same-size
    // placeholder so the layout doesn't jump while it's in the lightbox.
    var rect = mermaidDiv.getBoundingClientRect();
    var ph = document.createElement("div");
    ph.className = PLACEHOLDER_CLS;
    ph.style.width = rect.width + "px";
    ph.style.height = rect.height + "px";
    mermaidDiv.parentNode.replaceChild(ph, mermaidDiv);

    // Style the diagram for the lightbox: it keeps its intrinsic size,
    // and we apply a transform that scales/translates inside the stage.
    // No `will-change: transform` here — that would force the browser
    // to pre-rasterize at natural size and bilinear-scale to the target
    // (blur). bumpInteractive() turns it on only during wheel/drag and
    // releases it after 180ms idle so the SVG re-rasterizes crisp at
    // the final scale.
    mermaidDiv.style.cursor = "grab";
    mermaidDiv.style.transformOrigin = "center center";
    mermaidDiv.style.display = "block";
    stage.appendChild(mermaidDiv);
    currentMoved = mermaidDiv;
    currentPlaceholder = ph;

    overlay.classList.add("rb-lb-open");
    document.body.style.overflow = "hidden";

    // Compute the fit scale on the NEXT frame, after the browser has
    // laid out the moved diagram inside the lightbox stage (its
    // intrinsic size can differ from the article-context size).
    requestAnimationFrame(function () {
      var sw = stage.clientWidth || window.innerWidth * 0.95;
      var sh = stage.clientHeight || window.innerHeight * 0.9;
      var dw = mermaidDiv.clientWidth || rect.width || 1;
      var dh = mermaidDiv.clientHeight || rect.height || 1;
      // Start at ~75% of the available stage so the user has ample room
      // to zoom in further with the wheel.
      var fitScale = Math.min(sw / dw, sh / dh) * 0.75;
      fitScale = Math.max(1, Math.min(5, fitScale));
      overlay.__reset(fitScale);
      console.log(LOG, "lightbox: stage=" + sw + "x" + sh +
        " diagram=" + dw + "x" + dh + " fitScale=" + fitScale.toFixed(2));
    });
  }

  // ---------- Badge attachment ----------
  // Each mermaid div gets a sibling wrapper that contains both the div
  // and the badge. The badge does NOT live inside the .mermaid div — so
  // it never enters Material's content extraction.
  //
  // We only wrap once Material has rendered (the .mermaid div has size).
  function maybeWrap(el) {
    if (el.__rbBadged) return;
    // Render done? Material attaches a closed shadow DOM that gives the
    // div real layout dimensions. An empty div has clientHeight 0.
    if (!el.clientHeight && !el.clientWidth) return;
    el.__rbBadged = true;

    var parent = el.parentNode;
    if (!parent) return;

    var wrap = document.createElement("div");
    wrap.className = WRAP_CLS;
    parent.insertBefore(wrap, el);
    wrap.appendChild(el);

    var badge = document.createElement("button");
    badge.className = BADGE_CLS;
    badge.type = "button";
    badge.innerHTML = "&#x2922; Enlarge";
    badge.title = "Open at full size";
    badge.addEventListener("click", function (e) {
      e.preventDefault();
      e.stopPropagation();
      try { openLightboxWith(el); }
      catch (err) { console.error(LOG, "openLightboxWith threw:", err); }
    });
    wrap.appendChild(badge);

    // Clicking the diagram itself also opens.
    wrap.addEventListener("click", function (e) {
      if (e.target.closest("." + BADGE_CLS)) return;
      try { openLightboxWith(el); }
      catch (err) { console.error(LOG, "openLightboxWith threw:", err); }
    });

    console.log(LOG, "badge wrap attached for", el);
  }

  var MERMAID_SEL = "div.mermaid";

  function scan() {
    buildLightbox();
    document.querySelectorAll(MERMAID_SEL).forEach(maybeWrap);
  }

  // ---------- Observe the DOM ----------
  // Material creates a NEW <div class="mermaid"> via `e.replaceWith(r)`
  // — that fires a childList mutation on the parent. We catch it and
  // wait for the div to have a real size (shadow populated).
  var outer = new MutationObserver(function (mutations) {
    var sawMermaid = false;
    for (var i = 0; i < mutations.length && !sawMermaid; i++) {
      var m = mutations[i];
      if (m.addedNodes && m.addedNodes.length) {
        for (var j = 0; j < m.addedNodes.length; j++) {
          var n = m.addedNodes[j];
          if (n.nodeType !== 1) continue;
          if ((n.matches && n.matches(MERMAID_SEL)) ||
              (n.querySelector && n.querySelector(MERMAID_SEL))) {
            sawMermaid = true; break;
          }
        }
      }
    }
    if (sawMermaid) {
      // Wait a tick for the shadow root to populate, then try to wrap.
      scan();
      setTimeout(scan, 50);
      setTimeout(scan, 250);
    }
  });

  function bootstrap() {
    buildLightbox();
    scan();
    outer.observe(document.body || document.documentElement, {
      childList: true, subtree: true,
    });
    if (typeof window.document$ !== "undefined" && typeof window.document$.subscribe === "function") {
      window.document$.subscribe(function () {
        [50, 250, 1000, 3000].forEach(function (t) { setTimeout(scan, t); });
      });
    }
    // Also poll once a second for the first 10s to catch the moment
    // Material's mermaid render finishes (closed shadow attachment
    // doesn't always fire a useful mutation).
    var ticks = 0;
    var poll = setInterval(function () {
      scan();
      ticks++;
      if (ticks > 10) clearInterval(poll);
    }, 1000);
  }

  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", bootstrap);
  } else {
    bootstrap();
  }
})();
