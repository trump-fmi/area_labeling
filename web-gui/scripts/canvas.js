// canvas offset width, height
var CANVAS_OFFSET = [15, 110];
var DEBUG = true;
var BASELINE = 0.1;
var TEXTWIDTHRATIO = 0.6;

function setTextMeasures(ctx) {
    let txt = "QÄ"
    ctx.save();
    ctx.font = "10px monospace";
    TEXTWIDTHRATIO = ctx.measureText(txt).width / (10 * txt.length);
    ctx.restore();
}

function debug(message) {
    if (DEBUG) {
        console.log(message);
    }
}

class Canvas {
    constructor(element) {
        this.canvas = element;
        this.context = this.canvas.getContext("2d");

        setTextMeasures(this.context);

        this.windowFit();
        this.projector = new LinearProjector([0, this.width], [0, this.height], this.width, this.height);
    }

    getSize() {
        return [this.width, this.height]
    }

    setProjector(projector) {
        this.projector = projector;
    }

    windowFit() {
        this.resize(window.innerWidth - CANVAS_OFFSET[0], window.innerHeight - CANVAS_OFFSET[1]);
        this.redraw();
    }

    redraw() {
        this.drawBackground();
    }

    resize(width = this.width, height = this.height) {
        this.width = width;
        this.height = height;
        this.canvas.height = height;
        this.canvas.width = width;
    }

    drawBackground(color = "#DDDDDD") {
        this.context.fillStyle = color;
        this.context.fillRect(0, 0, this.width, this.height);

        if (DEBUG) {
            let ctx = this.context;
            ctx.save();
            ctx.beginPath()
            ctx.strokeStyle = "white";
            ctx.lineWidth = 0.51;
            ctx.moveTo(0.25 * this.width, this.height / 2);
            ctx.lineTo(0.75 * this.width, this.height / 2);
            ctx.moveTo(this.width / 2, 0.25 * this.height);
            ctx.lineTo(this.width / 2, 0.75 * this.height);
            ctx.stroke();
            ctx.restore()
        }
    }

    drawPolygon(polygon, style = "#000000", lineWidthOuter = 2, lineWidthInner = 1) {
        function draw_polyline(ctx, polyline) {
            ctx.beginPath()
            ctx.moveTo(...p.project(polyline[0]))
            for (var i = 1, size = polyline.length; i < size; i++) {
                ctx.lineTo(...p.project(polyline[i]));
            }
            ctx.stroke();
        };
        let ctx = this.context;
        let p = this.projector;
        let [outer, inners] = polygon;

        // draw the outer polygon
        ctx.save();
        ctx.strokeStyle = style;
        ctx.lineWidth = lineWidthOuter;
        draw_polyline(ctx, outer);
        ctx.restore();

        // draw the inner polygons
        ctx.save();
        ctx.strokeStyle = style;
        ctx.lineWidth = lineWidthInner;
        for (var i = 0, size = inners.length; i < size; i++) {
            debug(`Drawing ${inners[i]}`);
            draw_polyline(ctx, inners[i]);
        }
        ctx.restore();
    }

    /*
     * Draw an arc. The arc is defined as follows:
     * [
     *  [x, y],     // the arc center
     *  r,          // the arc radius
     *  from,       // the angle (in radians) where the arc starts
     *  to          // the angle (in radians) where the arc ends
     * ]
     */
    drawArc(arc, style = "#666666", lineWidth = 1, scale = false) {
        let ctx = this.context;
        let [c, r, a, b] = arc
        if (scale) {
            let p = this.projector;
            c = p.project(c);
            r = p.scale(r);
        }
        ctx.save();
        ctx.strokeStyle = style;
        ctx.lineWidth = lineWidth;
        ctx.beginPath()

        ctx.arc(...c, r, a, b);
        ctx.stroke();
        ctx.restore();
    }

    /*
     * Draw an curved label. This label is defined as follows:
     * [
     *  [x, y],     // the arc center
     *  [rad, RAD], // radius of the inner and OUTER arc
     *  [from, to], // angle from and to which the arc goes
     *  <label>     // the actual label
     * ]
     */
    drawCurvedLabel(curvedLabel) {
        let ctx = this.context;
        let p = this.projector;
        let [center, blRadius, fontSize, angle, label] = curvedLabel;
        let bl = p.scale(blRadius);
        let r = bl - BASELINE * fontSize;
        let R = r + fontSize;

        let angleRange = angle[1] - angle[0];
        let angleDelta = angleRange / label.length;

        ctx.save();
        ctx.font = fontSize + "px monospace";
        ctx.textAlign = "center";
        ctx.fillStyle = "black";
        ctx.translate(...p.project(center));
        ctx.rotate(Math.PI / 2);
        ctx.rotate(angle[0]);
        ctx.rotate(angleDelta / 2);
        for (let c of label) {
            ctx.fillText(c, 0, -r - BASELINE * fontSize);
            ctx.rotate(angleDelta);
        }
        ctx.restore();

        if (DEBUG) {
            this.drawArc([p.project(center), R, ...angle], "orange", 1);
            this.drawArc([p.project(center), r, ...angle], "green", 1);
        }
    }

    /*
     * Draw a disk label. The label is defined by:
     * [
     *  [x, y],         // the poi location
     *  rad,            // the radius of the poi disk
     *  size,           // the font size
     *  label           // the label itself
     * ]
     */
    drawDiskLabel(diskLabel) {
        let ctx = this.context;
        let p = this.projector;
        let [center, radius, size, label] = diskLabel;
        let c = p.project(center);
        let r = p.scale(radius)
        debug(`printing label "${label}" to position ${c} with size ${size}`);

        ctx.save();
        ctx.font = size + "px monospace";
        ctx.textAlign = "center";
        ctx.fillStyle = "black";
        ctx.translate(...c);
        ctx.fillText(label, 0, -size * BASELINE);
        ctx.restore();

        if (DEBUG) {
            this.drawArc([c, label.length * TEXTWIDTHRATIO * size / 2, 0, 2 * Math.PI], "green", 1);
        }
    }
}

function resize() {
    canvas.windowFit();
    test();
}

function refresh(data) {
    alert("Hello from refresh!");

    console.log(`${data}`);
}

var canvas;

function init() {
    canvas = new Canvas(document.getElementById('canvas'));
    document.getElementById("label_input").addEventListener('change', data_input);
    window.addEventListener("resize", resize, false);

    //    init_context();
    //    init_draw();
    //    init_canvas();

    test();
}

function test() {
    canvas.setProjector(new LinearProjector([0, 500], [-50, 150], ...canvas.getSize()));
    let area = new Area([[0, 0], [450, 150], [500, 100], [50, -50], [0, 0]],
        [
            [[100, 75], [400, 75], [400, 25], [100, 25]],
            [[300, 130], [200, 130], [250, 130], [250, 50]]
        ],
        new AreaLabel("QTESTÄ", [250, 0], 100, 20, -Math.PI / 2, 0)
    );
    area.draw(canvas);
    let poi = new PointOfInterest("TestDisk", [250, 130], 50, 20);
    poi.draw(canvas);
    poi = new PointOfInterest("__________", [250, 100], 60, 20);
    poi.draw(canvas);

    let obj = JSON.parse('{"type":"FeatureCollection","features":[{"type":"Feature","property":"blah"}, {"type":"Feature","property":"blah2"}]}');
}
/*
var context;
function init_context() {
    context = {
        ctx: document.getElementById('canvas').getContext("2d"),
        drawing: false,
        left_click: input_operation(function (x, y) {
        }),
        right_click: input_operation(function (x, y) {
        })
    };

    context["ctx"]
}

var draw;
function init_draw() {
    draw = {
        background: draw_operation(function () {
            alert("Test");
            let ctx = context["ctx"];
            ctx.fillStyle = "#DDDDDD";
            ctx.fillRect(0, 0, 750, 750);
        }),
        polygon: draw_operation(function (points) {
            let ctx = context["ctx"];
            if (points.length < 1)
                return;
            ctx.beginPath();
            ctx.strokeStyle = "#000000";
            ctx.moveTo(...points[0]);
            for (let i = 1; i < points.length; ++i) {
                ctx.lineTo(...points[i]);
            }
            ctx.stroke();
        }),
        polygons: draw_operation(function (polygons) {
            for (p of polygons) {
                draw["polygon"](p);
            }
            draw["polygon"](data["current_line"]);
        }),
        text_circular: draw_operation(function (curved_label) {
            let ctx = context["ctx"];
            let circle_text = curved_label.label;

            let h = curved_label.h;
            let H = 2 * h;
            let center = curved_label.c;
            let radius = curved_label.r;
            let inner_radius = radius - h;
            let start_angle = curved_label.a;
            let end_angle = curved_label.b;
            let angle_range = end_angle - start_angle;
            let angle_delta = angle_range / circle_text.length;

            ctx.font = H + "px monospace";
            ctx.textAlign = "center";
            ctx.fillStyle = "black";
            ctx.translate(...center);
            ctx.rotate(Math.PI / 2);
            ctx.rotate(start_angle);
            ctx.rotate(angle_delta / 2);
            let dy = - inner_radius - H / 5;
            let ct = circle_text;
            ///change for reverse
            if (Math.abs((start_angle + end_angle) / 2 - Math.PI / 2) < Math.PI / 2) {
                ct = ct.split("").reverse().join("");
                ctx.rotate(Math.PI);
                dy = inner_radius + 4 / 5 * H;
            }
            ///
            for (c of ct) {
                draw_letter_box(dy);
                ctx.fillText(c, 0, dy);
                ctx.rotate(angle_delta);
            }
        }),
    graph_edges: draw_operation(function (graph) {
        let ctx = context["ctx"];
        for (e of graph) {
            ctx.moveTo(...e[0]);
            ctx.lineTo(...e[1]);
            ctx.stroke();
        }
    }),
    label_line: draw_operation(function (curved_label) {
        let ctx = context["ctx"];
        let label_data = curved_label;
        ctx.beginPath();
        ctx.arc(...label_data.c, label_data.r, label_data.a, label_data.b);
        ctx.stroke();
    })
    }
}

function init_canvas() {
    canvas.oncontextmenu = e => { e.preventDefault(); e.stopPropagation(); };

    canvas.onmousedown = (mouse_event) => {
        let [x, y] = canvas_coords(mouse_event);
        console.log(mouse_event.button);
        if (mouse_event.button == 0) {
            //			left_click(x, y);
        }
        if (mouse_event.button == 2) {
            //          right_click"](x, y);
        }
    };

    // canvas.onclick = e => { context["drawing"] = !context["drawing"] };
    // canvas.onmousemove = e => { if (context["drawing"]) context["left_click"](...canvas_coords(e)); }
    alert("Test before!");
    draw["background"]();
}


function draw_operation(op) {
    let ctx = context["ctx"];
    return function () {
        ctx.save();
        op.apply(this, arguments);
        ctx.restore();
    };
}

function draw_letter_box(dy) {
    let ctx = context["ctx"];
    let h = data["label_data"].h;
    let H = 2 * h;
    let ASPECT = 1.63;
    let W = H / ASPECT;
    ctx.save();
    ctx.translate(0, dy);
    ctx.beginPath();
    ctx.moveTo(-W / 2, H / 5);
    ctx.lineTo(W / 2, H / 5);
    ctx.lineTo(W / 2, -H * 4 / 5);
    ctx.lineTo(-W / 2, -H * 4 / 5);
    ctx.lineTo(-W / 2, H / 5);
    ctx.closePath();
    ctx.stroke();
    ctx.restore();
}

function toRadian(degrees) {
    return 2 * Math.PI * degrees / 360.;
}

function input_operation(op) {
    return function () {
        op.apply(this, arguments);
        redraw();
    };
}

function canvas_coords(mouse_event) {
    let x = mouse_event.clientX - canvas.offsetLeft;
    let y = mouse_event.clientY - canvas.offsetTop;
    return [x, y];
}
*/