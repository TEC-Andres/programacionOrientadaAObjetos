// Defensive scroll-unlock for Material sidebar
(function () {
  try {
    document.addEventListener(
      "click",
      function (ev) {
        var el = ev.target;
        if (!(el instanceof Element)) return;
        var link = el.closest("a.md-nav__link, .md-sidebar a, a");
        if (!link) return;
        if (link.target && link.target !== "") return;
        if (document.body.hasAttribute("data-md-scrolllock")) {
          var top = parseInt(document.body.style.top || "0") || 0;
          document.body.removeAttribute("data-md-scrolllock");
          document.body.style.top = "";
          if (top)
            try {
              window.scrollTo(0, -top);
            } catch (e) {}
        }
      },
      true,
    );
  } catch (e) {}
})();
