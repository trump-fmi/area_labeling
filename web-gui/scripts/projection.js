class LinearProjector {
    constructor(x_range, y_range, width, height) {
        this.data_x_range = x_range;
        this.data_y_range = y_range;

        this.fitScale(width, height);
    }

    fitScale(width, height) {
        let x_diff = this.data_x_range[1] - this.data_x_range[0];
        let y_diff = this.data_y_range[1] - this.data_y_range[0];
        let x_scale = width / (x_diff);
        let y_scale = height / (y_diff);

        this.scaling = Math.min(x_scale, y_scale);
        // compute the offsets to fit the given width and height
        this.x_offset = -(width / this.scaling - x_diff) / 2 + this.data_x_range[0];
        this.y_offset = (height / this.scaling - y_diff) / 2 + this.data_y_range[1];
    }

    project(pos) {
        let [x, y] = pos;
        x = (x - this.x_offset) * this.scaling;
        y = -(y - this.y_offset) * this.scaling;
        return [x, y];
    }

    scale(length) {
        return length * this.scaling;
    }
}