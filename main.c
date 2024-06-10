#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "opencv2/opencv.hpp"


typedef struct Quadrant
{
    Rect bbox;
    int depth;
    bool leaf;
    Vecb colour;
    double detail;
    struct Quadrant* children[4];
}

// Function to create a new quadrant
Quadrant *
create_quadrant(Mat image, Rect bbox, int depth)
{
    Quadrant *quad = (Quadrant *)malloc(sizeof(Quadrant));
    quad->bbox = bbox;
    quad->depth = depth;
    quad->leaf = false;
    quad->detail = get_detail(image(bbox));
    quad->colour = average_color(image(bbox));
    for (int i = 0; i < 4; i++)
    {
        quad->children[i] = NULL;
    }
    return quad;
}

// Function to split a quadrant into four child quadrants
void split_quadrant(Quadrant *quad, Mat image)
{
    int middle_x = (quad->bbox.x + quad->bbox.width / 2);
    int middle_y = (quad->bbox.y + quad->bbox.height / 2);

    quad->children[0] = create_quadrant(image, Rect(quad->bbox.x, quad->bbox.y, middle_x - quad->bbox.x, middle_y - quad->bbox.y), quad->depth + 1);
    quad->children[1] = create_quadrant(image, Rect(middle_x, quad->bbox.y, quad->bbox.width - middle_x, middle_y - quad->bbox.y), quad->depth + 1);
    quad->children[2] = create_quadrant(image, Rect(quad->bbox.x, middle_y, middle_x - quad->bbox.x, quad->bbox.height - middle_y), quad->depth + 1);
    quad->children[3] = create_quadrant(image, Rect(middle_x, middle_y, quad->bbox.width - middle_x, quad->bbox.height - middle_y), quad->depth + 1);
}

int main(){

    printf("Hello World\n");

    return 0;
}