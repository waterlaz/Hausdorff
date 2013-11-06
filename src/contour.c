#include <stdlib.h>
#include <stdio.h>

#include "contour.h"
#include "math.h"

#include "array2d.h"

int min_int(int a, int b){
    if(a<b) return a;
    return b;
}

int min4_int(int a, int b, int c, int d){
    return min_int( min_int(a, b), min_int(c, d) );
}

int is_box_inside(contour_bounding_box_t* a, contour_bounding_box_t* b){
    return a->x1 >= b->x1 &&
           a->x1 <= b->x2 &&
           a->y1 >= b->y1 &&
           a->y1 <= b->y2 &&
           a->x2 >= b->x1 &&
           a->x2 <= b->x2 &&
           a->y2 >= b->y1 &&
           a->y2 <= b->y2;
}

contour_meta_t default_contour_meta(){
    contour_meta_t meta;
    meta.area = NULL;
    meta.perimeter = NULL;
    meta.bounding_box = NULL;
    return meta;
}

contour_t* alloc_contour(int n){
    DEF_ALLOC(res, contour_t);
    res->meta = default_contour_meta();
    res->n = n;
    res->points = (point_t*)malloc(sizeof(point_t)*n);
    return res;
}

void free_contour_meta(contour_meta_t meta){
    if(meta.area) free(meta.area);
    if(meta.perimeter) free(meta.perimeter);
    if(meta.bounding_box) free(meta.bounding_box);
}


void free_contour(contour_t* contour){
    free(contour->points);
    free_contour_meta(contour->meta);
    free(contour);
}

point_t inside_point(contour_t* c){
    point_t* p1 = c->points;
    point_t* p2 = c->points+1;
    double x = p1->y - p2->y;
    double y = -(p1->x - p2->x);
    double d = sqrt(x*x + y*y);
    x/=2*d;
    y/=2*d;
    point_t res;
    res.x = (p1->x+p2->x)/2+x;
    res.y = (p1->y+p2->y)/2+y;
    return res;
}

int is_point_inside_contour(point_t* p, contour_t* contour){
    int c = 0;
    FOR_CONTOUR_EDGES(contour, p1, p2){
        if((p1->y < p->y && p->y < p2->y)||
           (p1->y > p->y && p->y > p2->y)){
            double xt = (p2->x - p1->x)*(p->y - p1->y)/(p2->y - p1->y) + p1->x;
            if(xt < p->x) c++;
        }
    }
    return c%2;
}

double point_scalar(point_t a, point_t b){
    return a.x*b.x + a.y*b.y;
}
    
point_t point_plus(point_t a, point_t b){
    point_t res;
    res.x = a.x + b.x;
    res.y = a.y + b.y;
    return res;
}

point_t point_minus(point_t a, point_t b){
    point_t res;
    res.x = a.x - b.x;
    res.y = a.y - b.y;
    return res;
}

double point_abs(point_t a){
    return sqrt(point_scalar(a, a));
}

point_t point_mul(double x, point_t a){
    point_t res;
    res.x = x*a.x;
    res.y = x*a.y;
    return res;
}

double point_distance(point_t a, point_t b){
    /* return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y)); */
    return point_abs(point_minus(b, a));
}

contour_t* read_contour(char* file_name){
    int n;
    FILE* f = fopen(file_name, "r");
    if(f==NULL){ 
        printf("Failed openning file %s \n", file_name); 
        return NULL;
    }
    fscanf(f, "%d", &n);
    contour_t* contour = alloc_contour(n);
    int i;
    for(i=0; i<n; i++){
        double x, y;
        fscanf(f, "%lf %lf", &x, &y);
        contour->points[i].x = x;
        contour->points[i].y = y;
    }
    fclose(f);
    return contour;
}

int write_contour(contour_t* contour, char* file_name){
    FILE* f = fopen(file_name, "a");
    if(f==NULL){ 
        printf("Failed openning file %s for writing \n", file_name); 
        return 1;
    }
    fprintf(f, "%d\n", contour->n);
    int i;
    for(i=0; i<contour->n; i++){
        fprintf(f, "%lf %lf \n", contour->points[i].x, contour->points[i].y);
    }
    fclose(f);
    return 0;
}


/*
 * Contour finding
 */


struct point_color_pair{
    int x, y;
    int color;
};

/* pixel compare function */
int cmp_pcp(const void * a, const void * b){
    const struct point_color_pair* a1 = a;
    const struct point_color_pair* b1 = b;
    if (a1->color > b1->color) return 1;
    if (a1->color < b1->color) return -1;
    return 0;
}


/* compare contours by area, biggest area first */
int cmp_contour_area(const void * a, const void * b){
    const contour_t** a1 = a;
    const contour_t** b1 = b;
    return *((*b1)->meta.area) - *((*a1)->meta.area);
}



/* a double-linked list for contours */
struct component_list{
    struct component_list* prev;
    struct component_list* next;
    struct component_pixel* root; /* the root pixel in the disjoint-set corresponding to the component */
};

/* deletes a given component */
void delete_component(struct component_list* c){
    if(c->prev != NULL){
        c->prev->next = c->next;
    }
    if(c->next != NULL){
        c->next->prev = c->prev;
    }
    free(c);
}

struct component_list* create_component(struct component_list* l){
    struct component_list* t = (struct component_list*)malloc(sizeof(struct component_list));
    t->next = l->next;
    t->prev = l;
    l->next = t;
    if(t->next!=NULL) t->next->prev = t;
    return t;
}

/* a disjoint-set of pixels forming contours */
struct component_pixel{
  struct component_pixel* father;
  int size; /* this one is only meaningful for the root */
  struct component_list* component; /* this one is only meaningful for the root */
  int leftmost_x, leftmost_y; /* this one is only meaningful for the root */
  int is_read; /* this one is only meaningful for the root */
  int x, y;
};

/* finds the root pixel for the given pixel in the disjoint-set */
struct component_pixel* find_root(struct component_pixel* p){
    if(p->father==NULL) return p;
    struct component_pixel* r = find_root(p->father);
    /* move the node up the tree for the speedup of further searches */
    p->father = r;
    return r;
}


/* joins two subsets of disjoint set. The smaller subset is removed from the double-linked list of components */
void join_components(struct component_pixel* p1, struct component_pixel* p2){
    struct component_pixel* r1 = find_root(p1);
    struct component_pixel* r2 = find_root(p2);
    if(r1->size < r2->size){
        struct component_pixel* t = r1;
        r1 = r2;
        r2 = t;
    }
    r1->size+=r2->size;
    /* keeping the leftmost pixel coordinates */
    if(r1->leftmost_x > r2->leftmost_x){
        r1->leftmost_x = r2->leftmost_x;
        r1->leftmost_y = r2->leftmost_y;
    }
    /* This is a new region and therefore noone read it yet */
    r2->is_read = 0;
    r1->is_read = 0;
    
    r2->father = r1;
    delete_component(r2->component);
    r2->component = NULL;
}

/* */
void try_join_pixels(struct component_pixel* a, struct component_pixel* b){
    if(a==NULL || b==NULL) return;
    if(find_root(a)==find_root(b)) return;
    join_components(a, b);
}

struct component_pixel* create_pixel(struct component_list* list, int x, int y){
    struct component_pixel* res = (struct component_pixel*)malloc(sizeof(struct component_pixel));
    res->is_read=0;
    res->x = x;
    res->y = y;
    res->leftmost_x = x; 
    res->leftmost_y = y;
    res->size=1;
    struct component_list* component = create_component(list);
    component->root = res;
    res->component = component;
    res->father = NULL;
    return res;
}

contour_set_t* find_contours(image_t* img, int n_levels, int* level){
    int n_pixels = img->w * img->h;
    struct point_color_pair* pixels = (struct point_color_pair*)malloc(sizeof(struct point_color_pair)*n_pixels);
    int i, j;
    struct component_pixel*** field;
    ALLOC_2DARRAY(field, img->w, img->h, struct component_pixel*);
    /* copying all the pixel colors and coordinates into a 1d array pixels[] */
    struct point_color_pair* p = pixels;
    for(i=0; i<img->w; i++)
        for(j=0; j<img->h; j++){
            field[i][j] = NULL;
            p->x = i;
            p->y = j;
            p->color = img->img[i][j].r;
            p++;
        }
    /* sorting the pixels */
    qsort(pixels, n_pixels, sizeof(struct point_color_pair), cmp_pcp);
    
    struct component_list* components = (struct component_list*)malloc(sizeof(struct component_list));
    components->next = NULL;
    components->prev = NULL;

    
    /* Here we will store all the found contours before sorting them by area */
    int n_contours = 0;
    contour_t* contours[10000];
    
    p = pixels;

    for(i=0; i<n_levels; i++){
        int d = level[i];
        while(p<pixels+n_pixels && p->color<=d){
//            printf("color %d  at %d %d\n", p->color, p->x, p->y);
            field[p->x][p->y] = create_pixel(components, p->x, p->y);
  //          printf("made new pixel\n");
            if(p->x > 0) try_join_pixels(field[p->x][p->y], field[p->x - 1][p->y]);
            if(p->x < img->w - 1) try_join_pixels( field[p->x][p->y],  field[p->x + 1][p->y]);
            if(p->y > 0) try_join_pixels(field[p->x][p->y], field[p->x][p->y-1]);
            if(p->y < img->h - 1) try_join_pixels(field[p->x][p->y], field[p->x][p->y+1]);
            p++;
        }
       

        /* We start with c pointing to the first component and then go through all of them */
        struct component_list* c = components->next;
        while(c!=NULL){
            if(c->root->is_read){ 
                c = c->next;
                continue;
            }
            c->root->is_read=1;
            /* Here we temporarily store the contour: */
            int xs[10000]; 
            int ys[10000];
            int v_count = 0;
           
            /* The area bounded by the contour:  */ 
            int area = 0;

            int x0 = c->root->leftmost_x;
            int y0 = c->root->leftmost_y + 1;
            int x = x0;
            int y = y0;
            xs[v_count] = x;
            ys[v_count] = y;
            v_count++;

            /* Picking the initial direction of the contour traverse.
             * Since we start from the bottom left of the leftmost pixel in the contour,
             * the first step is possible */
            int dx = 0;
            int dy = -1;

            /* Updating the area */
            area += dx*y;

            x+=dx;
            y+=dy;
            #define tst_pixel(x, y) ((x)>=0 && (x)<img->w && (y)>=0 && (y)<img->h && field[x][y]!=NULL)
            #define turn_right(dx, dy) do{ int t=dx; dx=-dy; dy=t; }while(0) 
            #define turn_left(dx, dy) do{ int t=dx; dx=dy; dy=-t; }while(0) 
            #define right_pixelX(x, dx, dy) (x + min4_int(0, dx, dx-dy, -dy))
            #define right_pixelY(y, dx, dy) (y + min4_int(0, dy, dy+dx,  dx))
            #define can_move(x, y, dx, dy) (tst_pixel(right_pixelX(x, dx, dy), right_pixelY(y, dx, dy)))
            while(x!=x0 || y!=y0){
                img->img[min_int(x, img->w-1)][min_int(y, img->h-1)].r = 255;
                img->img[min_int(x, img->w-1)][min_int(y, img->h-1)].g = 0;
                img->img[min_int(x, img->w-1)][min_int(y, img->h-1)].b = 0; 
                xs[v_count] = x;
                ys[v_count] = y;
                v_count++;
                if(!can_move(x, y, dx, dy)){
                    turn_right(dx, dy);
                }else{
                    turn_left(dx, dy);
                    if(!can_move(x, y, dx, dy))
                        turn_right(dx, dy);
                }
                /* Updating the area */
                area += dx*y;
                /* Making the step */
                x+=dx;
                y+=dy;
            }
            
            /* ignore realy small contours */
            if(abs(area)>5){
                /* Here we already have our contour vertexes in xs[] and ys[]. 
                 * We need to create a new_contour and write the vertexes down */
                contour_t* new_contour = alloc_contour(v_count);
                new_contour->meta.area = ALLOC(double);
                *new_contour->meta.area = abs(area);
                DEF_ALLOC(box, contour_bounding_box_t);
                /* Prepare the bounding box of a contour */
                new_contour->meta.bounding_box = box;
                box->x1 = xs[0];
                box->y1 = ys[0];
                box->x2 = xs[0];
                box->y2 = ys[0];
                while(v_count--){
                    new_contour->points[v_count].x = xs[v_count];
                    new_contour->points[v_count].y = ys[v_count];
                    if(xs[v_count]<box->x1) box->x1 = xs[v_count];
                    if(ys[v_count]<box->y1) box->y1 = ys[v_count];
                    if(xs[v_count]>box->x2) box->x2 = xs[v_count];
                    if(ys[v_count]>box->y2) box->y2 = ys[v_count];
                }
                contours[n_contours] = new_contour;
                n_contours++;
//                write_contour(new_contour, "contours");
            }
            c = c->next; 
        }
    }

    int** draw_table;
    ALLOC_2DARRAY(draw_table, img->w+1, img->h+1, int);
    for(i=0; i<img->w+1; i++)
        for(j=0; j<img->h+1; j++){
            draw_table[i][j] = -1;
        }

    /* sort the contours by area from biggest to smallest */
    qsort(contours, n_contours, sizeof(contour_t*), cmp_contour_area);
    for(i=0; i<n_contours; i++){
        int k=i;
        while(k--){
            /* Check whether the i-th contour is inside the k-th contour */
            if(is_box_inside(contours[i]->meta.bounding_box, contours[k]->meta.bounding_box)){
                point_t p = inside_point(contours[i]);
                if(is_point_inside_contour(&p, contours[k])){
                    printf("%d inside %d\n", i, k);
                    break;
                }
            }
        }

    }
    
    FREE_2DARRAY(draw_table, img->w+1, img->h+1);

    while(components->next!=NULL)
        delete_component(components->next);
    free(pixels);
    FREE_2DARRAY(field, img->w, img->h);
    return NULL;
}

