c3 = {version: "1.0.0"};
c3.load = function(uri, async) {
  async = async || false;
  var req = new XMLHttpRequest();
  var onload = function() {
    if (!async || req.readyState == 4) {
      if (req.status == 200 || req.status == 0) {
        c3_init(JSON.parse(req.responseText));
//        c3_api();
      } else {
        alert("Error Loading C3 Data");
      }
    }
  };
  req.open('GET', uri, false);
  if (async) req.onreadystatechange = onload;
  req.send(null);
  if (!async) onload();
}

function c3_init(json) {
  var i, C, W, T, A, ccount, tcount;

  // parse colors
  c3.color = [];
  for (i=0; i<json.color.length; i+=3) {
    c3.color[i/3] = d3.lab(json.color[i], json.color[i+1], json.color[i+2]);
  }

  c3.count = json.v;    
  c3.pivot = d3.lab(json.pivot[0], json.pivot[1], json.pivot[2]);
}

c3.load("labcount.json");

var C = c3.color.length,
//    W = c3.terms.length,
    H = c3.count, //d3.range(0,C).map(c3.color.entropy),
    bg = { "xkcd":  "#fff" },
    exp = 1.15,
    ww = 200,
    sz = 5,
    qw = location.search.length ? ~~location.search.slice(1) : -1,
    MINCOUNT = 0.;


visualize("xkcd", H);

function visualize(name, H) {
  data = d3.range(0,C).map(function(i) {
    return { "chip": i, "s": H[i]}; //1/Math.pow(2,-H[i])};
  });

  function ss(d) {
    var s = d.s;
    var interp = ((s-minsal)/(maxsal-minsal));
      return 0. + (sz-1)*Math.pow(interp, exp);
  }

  var minsal = d3.min(data, function(d) { return d.s; }),
      maxsal = d3.max(data, function(d) { return d.s; }),
      maxqw = d3.max(data, function(d) { return qw<0 ? 1 : c3.count(d.chip, qw); }),
      xa = function(x) { return ww*(x+120)/240; },
      yb = function(y) { return ww - ww*(y+120)/240; };
  var L = [-1,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,110];

  var sum = data.reduce(function(a,b) { return a + b.s; }, 0);
  var thresh = -1000; // sum / data.length - 0.1;
  function filtered(lidx) {
    // filter to current L* bounds
    var a = data.filter(function(d) {
      var c = d.chip, ll = c3.color[c].L;
	return c3.count[c] > MINCOUNT &&  
          (ll >= L[lidx] && ll < L[lidx+1] && d.s > thresh);
    });
    // sort so larger squares are drawn first
    a.sort(function(a,b) { return b.s - a.s; });
    return a;
  }

  var div = d3.select("#"+name)
   .selectAll("div.color")
      .data(d3.range(0,L.length-1))
   .enter().append("div")
      .attr("class", "color");

  var svg = div.append("svg:svg")
      .style("width", 10+ww+"px")
      .style("height", ww+"px");

  div.append("div").text(function(d) { return "L* = "+d*5; });

  svg.append("svg:rect") 
	.attr("x",0)
	.attr("y",ww-30)
	.attr("width",30)
	.attr("height",30)
	.style("fill", c3.pivot);

  svg.selectAll("rect.swatch")
      .data(function(Lidx) { return filtered(Lidx); })
    .enter().append("svg:rect")
      .attr("class", "swatch")
      .attr("x", function(d) { return xa(c3.color[d.chip].a) + 0.5*(sz-ss(d)); })
      .attr("y", function(d) { return yb(c3.color[d.chip].b) + 0.5*(sz-ss(d)); })
      .attr("width", function(d) { return Math.floor(ss(d)); })
      .attr("height", function(d) { return Math.floor(ss(d)); })
      .style("stroke", "#ccc").style("stroke-width", 0.3)
      .style("fill", function(d,i) { return c3.color[d.chip]; })
      .style("fill-opacity", function(d,i) { return 1.; })
    .append("svg:title")
      .text(function(d) {
          return d.chip+" ("+c3.count[d.chip]+") "
      });
}
