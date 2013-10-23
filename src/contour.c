#include <stdlib.h>
#include <stdio.h>

#include "contour.h"
#include "math.h"

#include "array2d.h"

contour_t alloc_contour(int n){
    contour_t res;
    res.n = n;
    res.points = (point_t*)malloc(sizeof(point_t)*n);
    return res;
}

void free_contour(contour_t contour){
    free(contour.points);
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

int read_contour(contour_t* contour, char* file_name){
    int n;
    FILE* f = fopen(file_name, "r");
    if(f==NULL){ 
        printf("Failed openning file %s \n", file_name); 
        return 1;
    }
    fscanf(f, "%d", &n);
    *contour = alloc_contour(n);
    int i;
    for(i=0; i<n; i++){
        double x, y;
        fscanf(f, "%lf %lf", &x, &y);
        contour->points[i].x = x;
        contour->points[i].y = y;
    }
    fclose(f);
    return 0;
}

int write_contour(contour_t* contour, char* file_name){
    FILE* f = fopen(file_name, "w");
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
        struct component_list* c = components->next;
        while(c!=NULL){
            printf("%d %d    %d\n", c->root->leftmost_x, c->root->leftmost_y, c->root->size);
            int x0 = c->root->leftmost_x;
            int y0 = c->root->leftmost_y + 1;
            int x = x0;
            int y = y0;
            printf("%d %d\n", x, y);
            #define tst_pixel(x, y) ((x)>=0 && (x)<img->w && (y)>=0 && (y)<img->h && field[x][y]!=NULL)
            #define turn_right(dx, dy) if(dx==0&&dy==-1){ dx=1; dy=0; } else if(dx==1&& dy==0){ dx=0;dy=1; } else if(dx==0&&dy==1){ dx=-1;dy=0; } else if(dx==-1&&dy==0){dx=0;dy=-1;}
            #define turn_left(dx, dy) do{ turn_right(dx, dy);turn_right(dx, dy); turn_right(dx, dy);  }while(0)
            while(x!=x0 || y!=y0){
            }

            img->img[c->root->leftmost_x][c->root->leftmost_y].r=255;
            img->img[c->root->leftmost_x][c->root->leftmost_y].g=0;
            img->img[c->root->leftmost_x][c->root->leftmost_y].b=0;
            c = c->next; 
        }
        printf("########################\n");
    }

    while(components->next!=NULL)
        delete_component(components->next);
    free(pixels);
    FREE_2DARRAY(field, img->w, img->h);
    return NULL;
}
