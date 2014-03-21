#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "contour.h"
#include "math.h"

#include "array2d.h"

static int min_int(int a, int b){
    if(a<b) return a;
    return b;
}

static int min4_int(int a, int b, int c, int d){
    return min_int( min_int(a, b), min_int(c, d) );
}

static double max_double(double a, double b){
    if(a>b) return a;
    return b;
}

int is_box_inside(contour_bounding_box_t* a, contour_bounding_box_t* b){
    double e = 0.000001;
    return a->x1+e >= b->x1 &&
           a->x1-e <= b->x2 &&
           a->y1+e >= b->y1 &&
           a->y1-e <= b->y2 &&
           a->x2+e >= b->x1 &&
           a->x2-e <= b->x2 &&
           a->y2+e >= b->y1 &&
           a->y2-e <= b->y2;
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
    res->misc = NULL;
    res->n = n;
    res->points = (point_t*)malloc(sizeof(point_t)*n);
    return res;
}


contour_t* copy_contour(contour_t* c){
    contour_t* new_c = alloc_contour(c->n);
    memcpy(new_c->points, c->points, c->n*sizeof(point_t));
    new_c->is_bright = c->is_bright;
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
    
double point_vector(point_t a, point_t b){
    return a.x*b.y - b.x*a.y;
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


void scale_contour_by(contour_t* c, double x){
    FOR_CONTOUR_POINTS(c, p){
        *p = point_mul(x, *p);
    }
}

void scale_contour_to_1(contour_t* c){
    double max_x, max_y, min_x, min_y;
    max_x = c->points[0].x;
    max_y = c->points[0].y;
    min_x = c->points[0].x;
    min_y = c->points[0].y;
    FOR_CONTOUR_POINTS(c, p){
        if(p->x > max_x) max_x = p->x;
        if(p->x < min_x) min_x = p->x;
        if(p->y > max_y) max_y = p->y;
        if(p->y < min_y) min_y = p->y;
    }
    
    point_t d;
    d.x = min_x;
    d.y = min_y;

    double x = 1/max_double(max_x-min_x, max_y-min_y);

    FOR_CONTOUR_POINTS(c, p){
        *p = point_mul(x, point_minus(*p, d));
    }

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

/* This function recursively walks around the contur tree, printing the contours to files and building a tree file */
void write_contours_rec(int* n, int depth, FILE* f, contour_set_t* cur, char* dirname){
    /* increse the number of contours-nodes */
    (*n)++;
    /* print spaces accoding to depth */
    int i=depth;
    while(i--) fprintf(f, " ");
    /* finally print the file name with the contour contents */
    fprintf(f, "%d\n", *n);
    
    /* the actual filename is <dirname>/<*n> */
    char cont_file[2005];
    strncpy(cont_file, dirname, 1000);
    strncat(cont_file, "/", 1000);
    char* s = cont_file;
    while (*s) s++;
    snprintf(s, 1000, "%d", *n);
    /* write the contour to <cont_file> */
    write_contour(cur->node, cont_file);
    
    /* make a recursive call with depth incresed by one */
    for(i=0; i<cur->n; i++)
        write_contours_rec(n, depth+1, f, cur->children[i], dirname);

}

int write_contour_tree(contour_set_t* contours, char* dirname){
    if(mkdir(dirname, 0777)){
        printf("Error. Couldn't create dir %s", dirname);
        return 1;
    }
    char tree_file[2005];

    strncpy(tree_file, dirname, 1000);
    strncat(tree_file, "/tree", 1000);
    FILE* f = fopen(tree_file, "a");
   
    int n=0;

    write_contours_rec(&n, 0, f, contours, dirname);

    fclose(f);
    return 0;
}



contour_set_t* read_contur_tree(char* dirname){
    char tree_file[2005];
    strncpy(tree_file, dirname, 1000);
    strncat(tree_file, "/tree", 1000);
    FILE* f = fopen(tree_file, "r");
    contour_set_t* cs = alloc_contour_set();
    fclose(f);
}

int count_contours_in_set(contour_set_t* c){
    int n = c->n;
    int res = 1;
    while(n--){
        res+=count_contours_in_set(c->children[n]);
    }
    return res;
}

void write_contour_set_to_array(contour_set_t* c, contour_t*** a){
    **a = c->node;
    (*a)++;
    int n = c->n;
    while(n--){
        write_contour_set_to_array(c->children[n], a);
    }
}

contour_t** list_contour_set(contour_set_t* c, int* count){
    *count = count_contours_in_set(c);
    DEF_ALLOC_N(res, contour_t*, (*count));
    contour_t** p_res = res;
    write_contour_set_to_array(c, &p_res);
    return res;
}

void draw_contour(image_t* img, contour_t* contour, pixel_t color){
    FOR_CONTOUR_EDGES(contour, p1, p2){
        image_draw_line(img, (int)(p1->x+0.5), (int)(p1->y+0.5), (int)(p2->x+0.5), (int)(p2->y+0.5), color);
    }
}


/*
 * Contour finding
 */


struct point_color_pair{
    int x, y;
    int color;
};

/* pixel compare function */
static int cmp_pcp(const void * a, const void * b){
    const struct point_color_pair* a1 = a;
    const struct point_color_pair* b1 = b;
    if (a1->color > b1->color) return 1;
    if (a1->color < b1->color) return -1;
    return 0;
}


/* compare contours by area, biggest area first */
static int cmp_contour_set_area(const void * a, const void * b){
    const contour_set_t** a1 = a;
    const contour_set_t** b1 = b;
    return *((*b1)->node->meta.area) - *((*a1)->node->meta.area);
}



/* a double-linked list for contours */
struct component_list{
    struct component_list* prev;
    struct component_list* next;
    struct component_pixel* root; /* the root pixel in the disjoint-set corresponding to the component */
};

/* deletes a given component */
static void delete_component(struct component_list* c){
    if(c->prev != NULL){
        c->prev->next = c->next;
    }
    if(c->next != NULL){
        c->next->prev = c->prev;
    }
    free(c);
}

static struct component_list* create_component(struct component_list* l){
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
static struct component_pixel* find_root(struct component_pixel* p){
    if(p->father==NULL) return p;
    struct component_pixel* r = find_root(p->father);
    /* move the node up the tree for the speedup of further searches */
    p->father = r;
    return r;
}


/* joins two subsets of disjoint set. The smaller subset is removed from the double-linked list of components */
static void join_components(struct component_pixel* p1, struct component_pixel* p2){
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
static void try_join_pixels(struct component_pixel* a, struct component_pixel* b){
    if(a==NULL || b==NULL) return;
    if(find_root(a)==find_root(b)) return;
    join_components(a, b);
}

static struct component_pixel* create_pixel(struct component_list* list, int x, int y){
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


contour_set_t* alloc_contour_set(){
    DEF_ALLOC(res, contour_set_t);
    res->node = NULL;
    res->father = NULL;
    res->children = NULL;
    res->n = 0;
    return res;
}


void free_contour_set(contour_set_t* contour_set){
    int i;
    if(contour_set->node!=NULL) free_contour(contour_set->node);
    for(i=0; i<contour_set->n; i++){
        free_contour_set(contour_set->children[i]);
    }
    free(contour_set->children);
    free(contour_set);
}


struct component_pixel*** fff;

static int find_dark_contours(contour_set_t** contours, image_t* img, int n_levels, int* level){
    int n_pixels = img->w * img->h;
    DEF_ALLOC_N(pixels, struct point_color_pair, n_pixels);
    int i, j;
    /* Each field[][] belongs to one of the subsets in the disjoint set */
    struct component_pixel*** field;
    ALLOC_2DARRAY(field, img->w, img->h, struct component_pixel*);
    fff = field;
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
    
    DEF_ALLOC(components, struct component_list);
    components->next = NULL;
    components->prev = NULL;

    
    /* n_contoues is the number of currently found contours */
    int n_contours = 0;
    
    p = pixels;
    
    /* Here we temporarily store each the contour: */
    DEF_ALLOC_N(xs, int, (img->w+1)*(img->h+1));
    DEF_ALLOC_N(ys, int, (img->w+1)*(img->h+1));
    
    for(i=0; i<n_levels; i++){
        int d = level[i];
        while(p<pixels+n_pixels && p->color<=d){
            field[p->x][p->y] = create_pixel(components, p->x, p->y);
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
            int v_count = 0; /* the number of vertexes in the current contour */
           
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
            #define tst_pixel(x, y) ((x)>=0 && (x)<img->w && (y)>=0 && (y)<img->h && (/*printf("%d %d / %d %d\n", x, y, img->w, img->h),*/field[x][y]!=NULL))
            #define turn_right(dx, dy) do{ int t=dx; dx=-dy; dy=t; }while(0) 
            #define turn_left(dx, dy) do{ int t=dx; dx=dy; dy=-t; }while(0) 
            #define right_pixelX(x, dx, dy) (x + min4_int(0, dx, dx-dy, -dy))
            #define right_pixelY(y, dx, dy) (y + min4_int(0, dy, dy+dx,  dx))
            #define can_move(x, y, dx, dy) (tst_pixel(right_pixelX(x, dx, dy), right_pixelY(y, dx, dy)))
            while(x!=x0 || y!=y0){
                xs[v_count] = x;
                ys[v_count] = y;
                v_count++;
            if(field!=fff){
                
                printf("ololo\n");
                int i, j;
                while(v_count--){
                    printf("%d %d\n", xs[v_count], ys[v_count]);
                }
                exit(100);
            }
//                printf("%d\n", v_count);
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
                contours[n_contours] = alloc_contour_set();
                contours[n_contours]->node = new_contour;
                n_contours++;
                printf("%d\n", n_contours);
//                write_contour(new_contour, "contours");
            }
            c = c->next; 
        }
    }


    /* sort the contours by area from biggest to smallest */
    
    while(components->next!=NULL)
        delete_component(components->next);
    free(pixels);
    free(components);
    free(xs);
    free(ys);
    for(i=0; i<img->w; i++)
        for(j=0; j<img->h; j++)
            free(field[i][j]);
    FREE_2DARRAY(field, img->w, img->h);
    return n_contours;
}

contour_set_t* find_contours(image_t* img, int n_levels, int* level){
    DEF_ALLOC_N(contours, contour_set_t*, 1000000);
    int i;

    int n_dark = find_dark_contours(contours, img, n_levels, level);

    image_t* negative = image_negative(img);

    int reversed_level[1000];
    /* reverse the level array */
    for(i=0; i<n_levels; i++)
        reversed_level[i] = 255-level[n_levels-1-i];
    int n_bright = find_dark_contours(contours+n_dark, negative, n_levels, reversed_level);
    
    image_free(negative);

    int n_contours = n_dark+n_bright;

    for(i=0;i<n_dark;i++)
        contours[i]->node->is_bright = 0;
    for(i=n_dark;i<n_contours;i++)
        contours[i]->node->is_bright = 1;
    
    qsort(contours, n_contours, sizeof(contour_set_t*), cmp_contour_set_area);
    for(i=0; i<n_contours; i++){
        printf("finding home for %d\n", i);
        int k=i;
        while(k--) if(contours[k]!=NULL){
            /* Check whether the i-th contour is inside the k-th contour */
            if(is_box_inside(contours[i]->node->meta.bounding_box, contours[k]->node->meta.bounding_box)){
                /* If the areas are equal then this is obviously a duplicate */
                if(abs(*contours[i]->node->meta.area - *contours[k]->node->meta.area) < 0.3){
                    free_contour_set(contours[i]);
                    contours[i]=NULL;
                    break;
                }
                point_t p = inside_point(contours[i]->node);
                if(is_point_inside_contour(&p, contours[k]->node)){
                    contours[i]->father = contours[k];
                    contours[k]->n++;
//                    printf("%d inside %d \n", i, k);
                    break;
                }
            }
        }

    }

    for(i=0; i<n_contours; i++)if(contours[i]!=NULL){
        printf("=) %d\n", contours[i]->n);
        contours[i]->children = ALLOC_N(contour_set_t*, contours[i]->n);
        contours[i]->n = 0;
        /* if NULL then this must be root, so nothing more to do here */
        if(contours[i]->father==NULL) 
            continue;
        contours[i]->father->children[contours[i]->father->n] = contours[i];
        contours[i]->father->n++;
    }
        
    
    contour_set_t* res = contours[0];
    printf("done!\n");
    free(contours);
    return res;
}
