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