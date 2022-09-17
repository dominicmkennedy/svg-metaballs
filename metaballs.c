// Dominic Kennedy
// metaballs a c program for drawing metaballs via jgraph and bezier curves

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* constant for drawing circles with bezier curves */
const double C = 0.5519150244935105707435627;
/* MIN X, MAX X, MIN Y, MAX Y */
const double MIN_MAX_TABLE[4] = {-915, 1912, -1331, 2333};
const double SCALE = 1000.0;


// Usually just use the vec3 for colors
// so I just named the members RGB
struct vec3 {
    double r;
    double g;
    double b;
};

// Convienent to have a vec2 to store pos / velocity in 2d plane
struct vec2 {
    double x;
    double y;
};

// well known formula for approximating circles using bezier curves
// https://spencermortensen.com/articles/bezier-circle/
// instead of returning anything this function just prints instructions
// for jgraph to print the curve
void draw_circle(FILE *f, struct vec2 pos, double scale, struct vec3 color) {
    double x = pos.x;
    double y = pos.y;
    fprintf(f, "newline bezier poly pcfill %lf %lf %lf pts\n", color.r, color.g, color.b);
    fprintf(f, "%lf %lf\n", x, y-scale);
    fprintf(f, "%lf %lf   %lf %lf   %lf %lf\n", x+C*scale, y-scale,   x+scale,   y-C*scale, x+scale, y);
    fprintf(f, "%lf %lf   %lf %lf   %lf %lf\n", x+scale,   y+C*scale, x+C*scale, y+scale,   x,       y+scale);
    fprintf(f, "%lf %lf   %lf %lf   %lf %lf\n", x-C*scale, y+scale,   x-scale,   y+C*scale, x-scale, y);
    fprintf(f, "%lf %lf   %lf %lf   %lf %lf\n", x-scale,   y-C*scale, x-C*scale, y-scale,   x,       y-scale);
}

// euclidian distance for 2d space
double dist(struct vec2 p0, struct vec2 p1) {
    return sqrt(((p0.x-p1.x)*(p0.x-p1.x)) + ((p0.y-p1.y)*(p0.y-p1.y)));
}

// angle between two points in 2d space
double angle(struct vec2 p0, struct vec2 p1) {
    return atan2(p0.y-p1.y, p0.x-p1.x);
}

// gets the vector for a bezier handle
// given a circle position, radius, and handle angle
struct vec2 get_vector(struct vec2 p, double a, double r) {
    return (struct vec2) {.x=(p.x + r * cos(a)), .y=(p.y + r * sin(a))};
}

// draws the bezier curves for the meta part of the meta ball
// takes a file to write jgraph data to
// a point, radius and color for circle 1
// a point, radius and color for circle 2
void draw_metaball(FILE *f, struct vec2 point0, double radius0, struct vec2 point1, double radius1, struct vec3 color) {
    double d = dist(point0, point1);
    double max_dist = radius0 + radius1 * 2.5;

    // if the circles have no radius or if they're too far apart then return
    if (radius0==0 || radius1==0 || d>max_dist || d<=fabs(radius0-radius1)) return;

    double v = 0.5;
    double handle_size = 2.4;
    double u0 = 0;
    double u1 = 0;

    // calculate u0 and u1 if the circles are overlapping
    if (d < (radius0 + radius1)) {
        u0 = acos((radius0*radius0 + d*d - radius1*radius1) / (2*radius0*d));
        u1 = acos((radius1*radius1 + d*d - radius0*radius0) / (2*radius1*d));
    }

    double angle_between_points = angle(point1, point0);
    double max_spread = acos((radius0-radius1)/d);

    double angle0 = angle_between_points + u0 + (max_spread-u0) * v;
    double angle1 = angle_between_points - u0 - (max_spread-u0) * v;
    double angle2 = angle_between_points + M_PI - u1 - (M_PI-u1-max_spread) * v;
    double angle3 = angle_between_points - M_PI + u1 + (M_PI-u1-max_spread) * v;

    struct vec2 p0 = get_vector(point0, angle0, radius0);
    struct vec2 p1 = get_vector(point0, angle1, radius0);
    struct vec2 p2 = get_vector(point1, angle2, radius1);
    struct vec2 p3 = get_vector(point1, angle3, radius1);

    double d2 = fmin(v*handle_size, dist(p0,p2)/(radius0+radius1));
    d2 *= fmin(1, (d*2)/(radius0+radius1));

    double r0 = radius0 * d2;
    double r1 = radius1 * d2;

    // calculate handles for each of the angles
    struct vec2 h0 = get_vector(p0, angle0 - M_PI_2, r0);
    struct vec2 h1 = get_vector(p1, angle1 + M_PI_2, r0);
    struct vec2 h2 = get_vector(p2, angle2 + M_PI_2, r1);
    struct vec2 h3 = get_vector(p3, angle3 - M_PI_2, r1);

    struct vec2 edge = get_vector(point1, angle(point1, point0), radius1);

    // kind of hacky way to draw both lines, but it works so
    struct vec2 curve[10] = {p0, h0, h2, p2, edge, edge, p3, h3, h1, p1};

    fprintf(f, "newline bezier poly pcfill %lf %lf %lf pts\n", color.r, color.g, color.b);
    for (int i=0; i<10; ++i) fprintf(f, "%lf %lf\n", curve[i].x, curve[i].y);
}

// init a jgraph file
void init_graph(FILE *f, double scale) {
    // the coordinate system on the pdfs is a bit odd...
    // with the scale set to 1000 and no pdf cropping 
    // min and max coordinates that show up are as follows:
    // MIN X:  -915 MAX X:  2333
    // MIN Y: -1331 MAX Y:  2333
    fprintf(f, "newgraph\n");
    fprintf(f, "xaxis min 0 max %lf nodraw\n", scale);
    fprintf(f, "yaxis min 0 max %lf nodraw\n", scale);
}

// takes a circle and updates its position given it's velocity
// additionally it will reverse it's velocity to simulate a bounce
void update_pos(struct vec2 *p, double r, struct vec2 *v) {
    if (p->x+r > MIN_MAX_TABLE[1]) v->x *= -1;
    if (p->x-r < MIN_MAX_TABLE[0]) v->x *= -1;
    if (p->y+r > MIN_MAX_TABLE[3]) v->y *= -1;
    if (p->y-r < MIN_MAX_TABLE[2]) v->y *= -1;
    p->x += v->x;
    p->y += v->y;
}

struct vec2 random_pos_init() {
    int x_range = MIN_MAX_TABLE[1]-MIN_MAX_TABLE[0];
    int y_range = MIN_MAX_TABLE[3]-MIN_MAX_TABLE[2];

    return (struct vec2) {
        .x=(((rand()%60)+20)*x_range/100.0)+MIN_MAX_TABLE[0],
        .y=(((rand()%60)+20)*y_range/100.0)+MIN_MAX_TABLE[2]
    };
}

struct vec2 random_vel_init() {
    return (struct vec2) {
        .x=((rand()%100)-50),
        .y=((rand()%120)-60)
    };
}

int main(int argc, char **argv) {
    int num_frames = 50;
    if (argc == 3) { 
        num_frames = atoi(argv[1]);
        srand(atoi(argv[2])); 
    } else
    if (argc == 2) {
        num_frames = atoi(argv[1]);
        srand(time(NULL));
    } else {
        srand(time(NULL));
    }

    char fname[128];
    struct vec3 color = {.r=0, .g=0, .b=0};

    struct vec2 poses[6] = {
        random_pos_init(),random_pos_init(),
        random_pos_init(),random_pos_init(),
        random_pos_init(),random_pos_init(),
    };

    struct vec2 vels[6] = {
        random_vel_init(), random_vel_init(),
        random_vel_init(), random_vel_init(),
        random_vel_init(), random_vel_init(),
    };

    double rads[6] = {
        ((rand()%400)+50), ((rand()%50)+150),
        ((rand()%400)+50), ((rand()%50)+150),
        ((rand()%400)+50), ((rand()%50)+150),
    };

    for (int i=0; i<num_frames; ++i) {
        sprintf(fname, "./jgrs/frame%05d.jgr", i);

        FILE *f = fopen(fname, "w");

        init_graph(f, SCALE);

        for (int i=0; i<6; ++i) {
            update_pos(&poses[i], rads[i], &vels[i]);
            draw_circle(f, poses[i], rads[i], color);
        }

        for (int i=0; i<6; ++i) {
            for (int j=i+1; j<6; ++j) {
                draw_metaball(f, poses[i], rads[i], poses[j], rads[j], color);
            }
        }

        fclose(f);
    }

    return 0;
}
