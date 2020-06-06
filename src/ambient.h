#ifndef AMBIENT_H__
#define  AMBIENT_H__


#define MAX_SENSORS 10

#define CURSOR_ON  "\x1b[?25h"
#define CURSOR_OFF "\x1b[?25l"

#define BOLD   "\x1b[1m"
#define NORMAL "\x1b[0m"

#define MAGENTA  "\x1b[35m"
#define RED      "\x1b[31m"
#define GREEN    "\x1b[32m"
#define ORANGE   "\x1b[33m"
#define YELLOW   "\x1b[93m"
#define BLUE     "\x1b[34m"
#define LTBLUE   "\x1b[94m"
#define CYAN     "\x1b[36m"
#define WHITE    "\x1b[37m"
#define COLOR_RESET    "\x1b[0m"

#define ANSI_BG_GREEN       "\x1b[30;42m"
#define ANSI_BG_ORANGE      "\x1b[30;43m"
#define ANSI_BG_RED         "\x1b[37;41m"

#define HIDE_CURSOR  0
#define SHOW_CURSOR  1

__thread char ebuf[512]={0}; // for errno msgs, could be a lot smaller but for now 1/2k is fine
#define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))

typedef struct  {
    char hidraw[32];
    int thread_num;
    int display_only;
} t_info;

pthread_mutex_t display_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t plot_mutex = PTHREAD_MUTEX_INITIALIZER;


////////////////////////////////////////////////////////////////
////////////// user changeable variables ///////////////////////
////////////////////////////////////////////////////////////////

// temperatures that define the text color output
#define COLD   60.0
#define COOL   65.0
#define WARM   70.0
#define HOT    75.0

// if you change these make **sure** you include the 
// format char "%d" somewhere in the name string
char *by_day_file_fmt   = "by_day-%d.dat";
char *by_week_file_fmt  = "by_week-%d.dat";
char *by_month_file_fmt = "by_month-%d.dat";
int enable_graphing = true;
int enable_multi_plot = true; // most of the time this is fine, even for a single sensor

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////


    int have_enough_data = false;
    int do_display = false;
    int exit_flag = false;
    int sleep_time = 30; // in seconds, this really should be in units of 5 seconds
    int lines_per_day;
    int lines_per_week;
    int lines_per_month;

    void cursor_control(bool cur_cmd);
    void *sensor_thread(void *info);
    void update_display(int row_offset, char *new_temperature, char *secs, char *date);
    void append_to_file(int idx, char *fname, char *date_data, char *temperature_data);
    char *C_to_F(char *tempC);
    void get_date_data(char *secs, char *date, char *date_data);
    int trim_file(char *fname, int desired_lines);
    int line_count(char *fname);
    // not in use =>> void sync_to_next_sleep_time(void);
    void do_plot(void);
    // not in use =>> int wait(int fd, int seconds);
    int read_sensor(char *hidraw, char *sensor_data);
    int compare(const void *h1, const void *h2);
    int find_hidraw(char **hidraws, int *count);
    int find_hidraw2(char *hidraw_path); // << this will reliably find the right hidraw but its slow
    pthread_t start_thread(t_info info);

    char *local_time(char *format, char *result); // expects a 64 byte string

    char *get_word(char *str,int word_number);
    int cpy_file(const char *src, const char *dst, int allow_overwrite);
    int mv_file(const char *src, const char *dst, int allow_overwrite);
    char *ltrim(char *str);
    char *rtrim(char *str);
    char *trim(char *str);
    int create_plot_files(int sensor_count);

    const char plot_colors[10][16] = {"#408000", "#9900CC", "red", "orange", "blue", "green", "cyan", "violet", "brown", "yellow"};


// a complete single sensor pot script
char plot_single_scr[] =
         "#set term postscript eps enhanced colour\n"
         "set term pngcairo size 1800,1024 enhanced colour font \"Source Code Pro,10\" background rgb \"#e6f2ff\"\n"
         "# rgb \"#f3ffe6\"\n"
         "set output 'ambient-temperature.png'\n"
         "unset key\n"
         "set ylabel 'Temperature (Fahrenheit)'\n"
         "set border 3\n"
         "set tmargin  4\n"
         "set rmargin 8\n"
         "set xtics nomirror\n"
         "set ytics nomirror\n"
         "#set yrange [69:78.5]\n"
         "unset xlabel\n"
         "set grid ytics lw 1\n"
         "set xdata time\n"
         "set autoscale x\n"
         "set autoscale y\n"
         "#set xtics rotate by 30 offset -6.5,-3.7\n"
         "set multiplot title \"Ambient Temperature \".strftime(\"%%a %%b %%d %%H:%%M:%%S %%Y\", time(0)-7*3600)\n"
         "        set size 1.0,0.5\n"
         "        set origin 0.0,0.5\n"
         "                set title 'Monthly'\n"
         "                set xtics format \"%%H:%%M\\n%%m/%%d/%%y\"\n"
         "                set timefmt \"%%m/%%d/%%y_%%H:%%M:%%S\"\n"
         "                #set xtics 18000\n"
         "                #set xrange [\"00:00\":\"23:59\"]\n"
         "                #set xtics scale default rotate by 0 offset 0,0\n"
         "                plot 'by_month-1.dat' u 2:3 w lines lc \"#9900CC\" lw 1\n"
         "        set size 0.5,0.45\n"
         "        set origin 0,0.05\n"
         "                set title 'Weekly (bezier smoothed)'\n"
         "                set xtics format \"%%a\"\n"
         "                #set timefmt \"%%H:%%M:%%S\"\n"
         "                #set timefmt \"%%s\"\n"
         "                set timefmt \"%%m/%%d/%%y_%%H:%%M:%%S\"\n"
         "                set xtics 86400\n"
         "                plot 'by_week-1.dat' u 2:3 w lines lc \"blue\" smooth bezier\n"
         "        set origin 0.515,0.05\n"
         "                set title 'Daily'\n"
         "                #set xtics format \"%%m/%%d/%%y %%H\"\n"
         "                #set timefmt \"%%s\"\n"
         "                set timefmt \"%%m/%%d/%%y_%%H:%%M:%%S\"\n"
         "                set format x \"%%a\\n%%H:%%M\"\n"
         "                set xtics 14400*1.25\n"
         "                plot 'by_day-1.dat' u 2:3 w lines lc \"#408000\"\n"
         "\n";

// this is for building the multi-plot
const char multi_plot_part1[] =
          "#\n"
          "# ambient Temper Gold v3.1 multi-sensor plot file for use with gnuplot\n"
          "#\n"
          "#set term postscript eps enhanced colour\n"
          "set term pngcairo size 1800,1024 enhanced colour font \"Source Code Pro,10\" background rgb \"#e6f2ff\"\n"
          "# rgb \"#f3ffe6\"\n"
          "set output 'ambient-temperature.png'\n"
          "unset key\n"
          "set key left top \n"
          "set ylabel 'Temperature (Fahrenheit)'\n"
          "set border 3\n"
          "set tmargin  4\n"
          "set rmargin 8\n"
          "set xtics nomirror\n"
          "set ytics nomirror\n"
          "#set yrange [69:78.5]\n"
          "unset xlabel\n"
          "set grid ytics lw 1\n"
          "set xdata time\n"
          "set autoscale x\n"
          "set autoscale y\n"
          "#set xtics rotate by 30 offset -6.5,-3.7\n"
          "set multiplot title \"Ambient Temperature \".strftime(\"%a %b %d %H:%M:%S %Y\", time(0)-7*3600)\n"
          "        set size 1.0,0.5\n"
          "        set origin 0.0,0.5\n"
          "                set title 'Monthly'\n"
          "                set xtics format \"%H:%M\\n%m/%d/%y\"\n"
          "                set timefmt \"%m/%d/%y_%H:%M:%S\"\n"
          "                set ytics 2\n"
          "                #set xrange [\"00:00\":\"23:59\"]\n"
          "                #set xtics scale default rotate by 0 offset 0,0\n"
          "                plot 'by_month-1.dat' u 2:3 title \"sensor 1\" w lines lc \"#408000\" lw 1";
const char multi_plot_part2[] = ", 'by_month-%d.dat' u 2:3 title \"sensor %d\"  w lines lc \"%s\" lw 1";
const char multi_plot_part3[] =
          "        set size 0.5,0.45\n"
          "        set origin 0,0.05\n"
          "                set title 'Weekly (bezier smoothed)'\n"
          "                set xtics format \"%a\"\n"
          "                #set timefmt \"%H:%M:%S\"\n"
          "                #set timefmt \"%s\"\n"
          "                set timefmt \"%m/%d/%y_%H:%M:%S\"\n"
          "                set xtics 86400\n"
          "                plot 'by_week-1.dat' u 2:3 title \"sensor 1\" w lines lc \"#408000\" smooth bezier";
const char multi_plot_part4[] = ", 'by_week-%d.dat' u 2:3 title \"sensor %d\" w lines lc \"%s\" smooth bezier";
const char multi_plot_part5[] =
          "        set origin 0.515,0.05\n"
          "                set title 'Daily'\n"
          "                #set xtics format \"%m/%d/%y %H\"\n"
          "                #set timefmt \"%s\"\n"
          "                set timefmt \"%m/%d/%y_%H:%M:%S\"\n"
          "                set format x \"%a\\n%H:%M\"\n"
          "                set xtics 14400*1.25\n"
          "                plot 'by_day-1.dat' u 2:3 title \"sensor 1\" w lines lc \"#408000\"";
const char multi_plot_part6[] = ", 'by_day-%d.dat' u 2:3 title \"sensor %d\" w lines lc \"%s\"";

#endif
