var data;
function init_data() {
    data = {
        circle_text: "Hello World",
        label_data: { c: [200, 200], r: 100, h: 20, a: Math.PI / 4, b: Math.PI * 3 / 2 },
        polygons: [],
        current_line: [],
        graph_edges: []
    }
}

var context;
function init_context() {
    context = {
        ctx: document.getElementById('canvas').getContext("2d"),
        drawing: false,
        left_click: input_operation(function (x, y) {
            data["current_line"].push([x, y]);
        }),
        right_click: input_operation(function (x, y) {
            data["current_line"].push(data["current_line"][0]);
            data["polygons"].push(data["current_line"]);
            data["current_line"] = [];
            skeleton_points();
            labelling();
        })
    }
}

var buttons;
function init_buttons() {
    buttons = {
        skeleton_button: document.getElementById('skeleton-button'),
        label_button: document.getElementById('label-button')
    }

    buttons["skeleton_button"].onclick = skeleton_points;
    buttons["label_button"].onclick = labelling;
}

var input;
function init_input() {
    input = {
        circle_text_input: document.getElementById('circle-text-input'),
        setCircleText: input_operation(function (text) {
            console.log(text);
            data["circle_text"] = text;
            labelling();
        }),
        setGraphEdges: input_operation(function (edges) {
            data["graph_edges"] = edges;
        }),
        setLabelData: input_operation(function (ldata) {
            console.log(ldata);
            data["label_data"] = ldata;
        }),
    }
    input["circle_text_input"].oninput = (e) => { input["setCircleText"](e.target.value) };
}



var draw;
function init_draw() {
    draw = {
        background: draw_operation(function () {
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
        polygons: draw_operation(function () {
            for (p of data["polygons"]) {
                draw["polygon"](p);
            }
            draw["polygon"](data["current_line"]);
        }),
        text_circular: draw_operation(function () {
            let ctx = context["ctx"];
            let circle_text = data["circle_text"];
            let label_data = data["label_data"];

            let h = label_data.h;
            let H = 2 * h;
            let center = label_data.c;
            let radius = label_data.r;
            let inner_radius = radius - h;
            let start_angle = label_data.a;
            let end_angle = label_data.b;
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
                ctx.fillText(c, /* dx= */0, dy);
                ctx.rotate(angle_delta);
            }
        }),
        graph_edges: draw_operation(function () {
            let ctx = context["ctx"];
            for (s of data["graph_edges"]) {
                ctx.moveTo(...s[0]);
                ctx.lineTo(...s[1]);
                ctx.stroke();
            }
        }),
        label_line: draw_operation(function () {
            let ctx = context["ctx"];
            let label_data = data["label_data"];
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
            context["right_click"](x, y);
        }
    };

    canvas.onclick = e => { context["drawing"] = !context["drawing"] };
    canvas.onmousemove = e => { if (context["drawing"]) context["left_click"](...canvas_coords(e)); }
}


function init() {
    init_context();
    init_buttons();
    init_data();
    init_input();
    init_draw();

    init_canvas();

    redraw();
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

function redraw() {
    draw["background"]();
    draw["polygons"]();
    draw["graph_edges"]();
    draw["text_circular"]();
    draw["label_line"]();
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

function skeleton_points() {
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "/skeleton");
    xhr.onload = () => {
        if (xhr.status != 200) {
            console.log('error occured', xhr.status);
            return;
        }
        input["setGraphEdges"](JSON.parse(xhr.responseText));
    };
    xhr.send(JSON.stringify([...data["polygons"], data["current_line"]]));
}


function labelling() {
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "/label");
    xhr.onload = () => {
        if (xhr.status != 200) {
            console.log('error occured', xhr.status);
            return;
        }
        input["setLabelData"](JSON.parse(xhr.responseText));
    }
    xhr.send(JSON.stringify({
        'poly': [...data["polygons"], data["current_line"]],
        'text': data["circle_text"]
    }));
}
