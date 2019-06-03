
class PointOfInterest {
    constructor(label, pos, radius, font_size) {
        this.label = label;
        this.pos = pos;
        this.radius = radius;
        this.font_size = font_size;
    }

    draw(canvas) {
        canvas.drawDiskLabel([this.pos, this.radius, this.font_size, this.label]);
    }
}


class AreaLabel {
    constructor(label, center, baselineRadius, fontSize, angleStart, angleEnd) {
        this.label = label;
        this.center = center;
        this.blRad = baselineRadius;
        this.fontSize = fontSize;
        this.angleStart = angleStart;
        this.angleEnd = angleEnd;
    }

    draw(canvas) {
        canvas.drawCurvedLabel([this.center, this.blRad, this.fontSize, [this.angleStart, this.angleEnd], this.label]);
    }
}


class Area {
    constructor(outer, inners, label) {
        this.outer = outer;
        this.inners = inners;
        this.name = name;
        this.label = label;
    }

    draw(canvas) {
        this.drawBorder(canvas);
        this.drawLabel(canvas);
    }

    drawBorder(canvas) {
        canvas.drawPolygon([this.outer, this.inners])
    }

    drawLabel(canvas) {
        this.label.draw(canvas);
    }
}


class AreaPOI {
    constructor(area, poi) {
        this.area = area;
        this.poi = poi;
    }
}