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