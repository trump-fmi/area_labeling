// function init() {
var ctx = document.getElementById('canvas').getContext("2d");

var skeleton_button = document.getElementById('skeleton-button');
var label_button = document.getElementById('label-button');
var circle_text_input = document.getElementById('circle-text-input');

let polygons = [];
let current_line = [];

let circle_text = "Hello World!";

let graph_edges = [];
let label_data = { c: [200, 200], r: 100, h: 20, a: Math.PI / 4, b: Math.PI * 3 / 2 };

let drawing = false;

var draw_background = draw_operation(function () {
    ctx.fillStyle = "#DDDDDD";
    ctx.fillRect(0, 0, 750, 750);
});

var draw_polygon = draw_operation(function (points) {
    if (points.length < 1)
        return;
    ctx.beginPath();
    ctx.strokeStyle = "#000000";
    ctx.moveTo(...points[0]);
    for (let i = 1; i < points.length; ++i) {
        ctx.lineTo(...points[i]);
    }
    ctx.stroke();
});

var draw_polygons = draw_operation(function () {
    for (p of polygons) {
        draw_polygon(p);
    }
    draw_polygon(current_line);
});

var draw_text_circular = draw_operation(function () {
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
});

var draw_graph_edges = draw_operation(function () {
    for (s of graph_edges) {
        ctx.moveTo(...s[0]);
        ctx.lineTo(...s[1]);
        ctx.stroke();
    }
});

var draw_label_line = draw_operation(function () {
    ctx.beginPath();
    ctx.arc(...label_data.c, label_data.r, label_data.a, label_data.b);
    ctx.stroke();
});

var left_click = input_operation(function (x, y) {
    current_line.push([x, y]);
});

var right_click = input_operation(function (x, y) {
    current_line.push(current_line[0]);
    polygons.push(current_line);
    current_line = [];
    skeleton_points();
    labelling();
});

var setGraphEdges = input_operation(function (edges) {
    graph_edges = edges;
});

redraw();

var setLabelData = input_operation(function (ldata) {
    console.log(ldata);
    label_data = ldata;
});

canvas.oncontextmenu = e => { e.preventDefault(); e.stopPropagation(); };

canvas.onmousedown = (mouse_event) => {
    let [x, y] = canvas_coords(mouse_event);
    console.log(mouse_event.button);
    if (mouse_event.button == 0) {
        //			left_click(x, y);
    }
    if (mouse_event.button == 2) {
        right_click(x, y);
    }
};

canvas.onclick = e => { drawing = !drawing };
canvas.onmousemove = e => { if (drawing) left_click(...canvas_coords(e)); }

skeleton_button.onclick = skeleton_points;
label_button.onclick = labelling;

var setCircleText = input_operation(function (text) {
    console.log(text);
    circle_text = text;
    labelling();
});
circle_text_input.oninput = (e) => { setCircleText(e.target.value) };
// }

function draw_operation(op) {
    return function () {
        ctx.save();
        op.apply(this, arguments);
        ctx.restore();
    };
}

function draw_letter_box(dy) {
    let h = label_data.h;
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
    draw_background();
    draw_polygons();
    draw_graph_edges();
    draw_text_circular()
    draw_label_line();
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
        setGraphEdges(JSON.parse(xhr.responseText));
    };
    xhr.send(JSON.stringify([...polygons, current_line]));
}


function labelling() {
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "/label");
    xhr.onload = () => {
        if (xhr.status != 200) {
            console.log('error occured', xhr.status);
            return;
        }
        setLabelData(JSON.parse(xhr.responseText));
    }
    xhr.send(JSON.stringify({
        'poly': [...polygons, current_line],
        'text': circle_text
    }));
}
