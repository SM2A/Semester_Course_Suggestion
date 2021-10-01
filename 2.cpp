#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>

#define COLUMN_SEPARATOR ','
#define MULTI_SEPARATOR '-'
#define HOUR_MINUTE_SEPARATOR ':'
#define TIME_SEPARATOR '/'
#define ARGV_COURSES_INDEX 1
#define ARGV_GRADES_INDEX 2
#define NOT_FOUND -1
#define PASS_GRADE 10
#define NO_PREREQUISITES 0
#define HIGH_GRADE 17
#define MAX_UNIT_FOR_LESS_17 20
#define MAX_UNIT_FOR_UP_17 24
#define TIME_CONVERTER 100
#define FIRST_INDEX 0

using namespace std;

enum time_mode {start_time, end_time};

struct grade {
    int id;
    float grade;
};
typedef struct grade course_grade;
typedef vector<course_grade> student_grades;

struct time {
    string day_of_week;
    int start_hour;
    int start_minute;
    int end_hour;
    int end_minute;
};
typedef struct time course_time;
typedef vector<course_time> course_schedule;
typedef vector<int> course_prerequisites;
struct course_data {
    int id;
    string name;
    int units;
    course_schedule schedule;
    course_prerequisites prerequisites;
};
typedef struct course_data course;
typedef vector<course> student_courses;

struct argument_data {
    string courses;
    string grades;
};
typedef struct argument_data arguments;

student_courses read_course_data(string path);
student_grades read_grades_data(string path);
arguments get_arguments(char *argv[]);
course_schedule get_course_schedule(string schedule);
course_prerequisites get_course_prerequisites(string prerequisites);
student_courses find_allowed_courses(student_courses courses, student_grades grades);
float find_course_grade_by_id(int id, student_grades grades);
bool sort_course_by_name(course course1, course course2);
bool sort_course_by_unit(course course1, course course2);
bool sort_grade_by_id(course_grade grade1, course_grade grade2);
void sort_grade_id(student_grades &grades);
void sort_name_unit(student_courses &courses);
bool sort_grade_by_mark(course_grade grade1, course_grade grade2);
void print_courses_id(student_courses courses);
float calculate_average_grade(student_grades grades, student_courses courses);
course find_course_by_id(int id, student_courses courses);
student_grades filter_grades(student_grades grades);
student_courses suggested_courses(student_courses allowed_courses, float average_grade);
bool check_interference(int index, student_courses courses);
int time_convert(course_time class_time, time_mode mode);

int main(int argc, char *argv[]) {

    arguments files_path = get_arguments(argv);
    student_grades grades = read_grades_data(files_path.grades);
    student_courses courses = read_course_data(files_path.courses);
    student_courses all_allowed_courses = find_allowed_courses(courses, grades);
    sort(all_allowed_courses.begin(), all_allowed_courses.end(), sort_course_by_unit);
    sort_name_unit(all_allowed_courses);
    float average_grade = calculate_average_grade(grades, courses);
    student_courses next_term_courses = suggested_courses(all_allowed_courses, average_grade);
    sort(next_term_courses.begin(), next_term_courses.end(), sort_course_by_name);
    print_courses_id(next_term_courses);

    return 0;
}

arguments get_arguments(char *argv[]) {

    arguments file_names;
    file_names.courses = argv[ARGV_COURSES_INDEX];
    file_names.grades = argv[ARGV_GRADES_INDEX];

    return file_names;
}

student_grades read_grades_data(string path) {

    ifstream grades_file(path);
    student_grades grades;
    string input_content;

    string column_name;
    getline(grades_file, column_name);

    while (getline(grades_file, input_content)) {

        course_grade grade;
        stringstream stream;
        string row_data;
        stream << input_content << endl;

        getline(stream, row_data, COLUMN_SEPARATOR);
        grade.id = stoi(row_data);
        getline(stream, row_data);
        grade.grade = stof(row_data);

        grades.push_back(grade);
    }

    return grades;
}

student_courses read_course_data(string path) {

    ifstream course_file(path);
    string input_content;
    student_courses courses;

    string column_name;
    getline(course_file, column_name);

    while (getline(course_file, input_content)) {

        course course_info;
        string row_data;
        stringstream stream;
        stream << input_content << endl;

        getline(stream, row_data, COLUMN_SEPARATOR);
        course_info.id = stoi(row_data);
        getline(stream, row_data, COLUMN_SEPARATOR);
        course_info.name = row_data;
        getline(stream, row_data, COLUMN_SEPARATOR);
        course_info.units = stoi(row_data);
        getline(stream, row_data, COLUMN_SEPARATOR);
        course_info.schedule = get_course_schedule(row_data);
        getline(stream, row_data);
        course_info.prerequisites = get_course_prerequisites(row_data);

        courses.push_back(course_info);
    }

    return courses;
}

course_schedule get_course_schedule(string schedule) {

    stringstream schedule_stream;
    schedule_stream << schedule << endl;
    course_schedule schedule_data;
    string single_day;

    while (getline(schedule_stream, single_day, TIME_SEPARATOR)) {

        course_time class_time;
        stringstream day_stream;
        day_stream << single_day << endl;
        string day_data;

        getline(day_stream, day_data, MULTI_SEPARATOR);
        class_time.day_of_week = day_data;

        getline(day_stream, day_data, MULTI_SEPARATOR);
        stringstream time_stream;
        time_stream << day_data << endl;

        getline(time_stream, day_data, HOUR_MINUTE_SEPARATOR);
        class_time.start_hour = stoi(day_data);
        getline(time_stream, day_data, MULTI_SEPARATOR);
        class_time.start_minute = stoi(day_data);

        time_stream.clear();
        getline(day_stream, day_data, TIME_SEPARATOR);
        time_stream << day_data << endl;

        getline(time_stream, day_data, HOUR_MINUTE_SEPARATOR);
        class_time.end_hour = stoi(day_data);
        getline(time_stream, day_data, TIME_SEPARATOR);
        class_time.end_minute = stoi(day_data);

        schedule_data.push_back(class_time);
    }

    return schedule_data;
}

course_prerequisites get_course_prerequisites(string prerequisites) {

    course_prerequisites prerequisites_ids;
    stringstream prerequisites_stream;
    prerequisites_stream << prerequisites << endl;
    string prerequisites_id;

    while (getline(prerequisites_stream, prerequisites_id, MULTI_SEPARATOR))
        prerequisites_ids.push_back(stoi(prerequisites_id));

    return prerequisites_ids;
}

student_courses find_allowed_courses(student_courses courses, student_grades grades) {

    student_courses allowed_courses_data;

    for (int index = 0; index < courses.size(); ++index) {

        course course_info;
        bool check_all_prerequisites = true;
        bool have_prerequisites = false;
        if (find_course_grade_by_id(courses[index].id, grades) >= PASS_GRADE) continue;

        if (courses[index].prerequisites[FIRST_INDEX] == NO_PREREQUISITES) {

            course_info.name = courses[index].name;
            course_info.id = courses[index].id;
            course_info.units = courses[index].units;
            course_info.prerequisites = courses[index].prerequisites;
            course_info.schedule = courses[index].schedule;
            allowed_courses_data.push_back(course_info);
            continue;
        }

        for (int prerequisites = 0; prerequisites < courses[index].prerequisites.size(); ++prerequisites) {
            have_prerequisites = true;
            if (courses[index].prerequisites[prerequisites] != NO_PREREQUISITES) {

                check_all_prerequisites =
                        (check_all_prerequisites)
                        && (find_course_grade_by_id(courses[index].prerequisites[prerequisites], grades) >= PASS_GRADE);
            }

        }
        if (check_all_prerequisites && have_prerequisites) {
            course_info.name = courses[index].name;
            course_info.id = courses[index].id;
            course_info.units = courses[index].units;
            course_info.prerequisites = courses[index].prerequisites;
            course_info.schedule = courses[index].schedule;
            allowed_courses_data.push_back(course_info);
        }
    }

    return allowed_courses_data;
}

float find_course_grade_by_id(int id, student_grades grades) {

    vector<float> course_grade;
    for (int index = 0; index < grades.size(); ++index) {
        if (id == grades[index].id) course_grade.push_back(grades[index].grade);
    }

    if (course_grade.empty())return NOT_FOUND;

    sort(course_grade.begin(), course_grade.end());
    return course_grade[course_grade.size() - 1];
}

course find_course_by_id(int id, student_courses courses) {

    for (int index = 0; index < courses.size(); ++index) {
        if (courses[index].id == id) return courses[index];
    }
}

bool sort_course_by_name(course course1, course course2) {

    return course2.name > course1.name;
}

bool sort_course_by_unit(course course1, course course2) {

    return course1.units > course2.units;
}

bool sort_grade_by_id(course_grade grade1, course_grade grade2) {

    return grade2.id > grade1.id;
}

bool sort_grade_by_mark(course_grade grade1, course_grade grade2) {

    return grade2.grade > grade1.grade;
}

void sort_name_unit(student_courses &courses) {

    int begin = 0;
    int end = 0;

    for (int index = 0; index < courses.size(); ++index) {

        if (courses[begin].units == courses[end].units) {
            end++;
            if (index != (courses.size() - 1))continue;
        }
        sort(courses.begin() + begin, courses.begin() + end, sort_course_by_name);
        begin = end;
        end++;
    }
}

void sort_grade_id(student_grades &grades) {

    int begin = 0;
    int end = 0;

    for (int index = 0; index < grades.size(); ++index) {

        if (grades[begin].id == grades[end].id) {
            end++;
            if (index != (grades.size() - 1))continue;
        }
        sort(grades.begin() + begin, grades.begin() + end, sort_grade_by_mark);
        begin = end;
        end++;
    }
}

void print_courses_id(student_courses courses) {

    for (int index = 0; index < courses.size(); ++index) {
        cout << courses[index].id << endl;
    }
}

float calculate_average_grade(student_grades grades, student_courses courses) {

    float average_grade = 0;
    float sum = 0;
    int weight = 0;
    student_grades filtered_grades = filter_grades(grades);

    for (int index = 0; index < filtered_grades.size(); ++index) {
        sum += ((filtered_grades[index].grade) * (float) (find_course_by_id(filtered_grades[index].id, courses)).units);
        weight += find_course_by_id(filtered_grades[index].id, courses).units;
    }
    average_grade = sum / (float)weight;

    return average_grade;
}

student_grades filter_grades(student_grades grades) {

    sort(grades.begin(), grades.end(), sort_grade_by_id);
    sort_grade_id(grades);

    for (int index = 0; index < grades.size(); ++index) {
        if (grades[index].id == grades[index + 1].id)
            grades.erase(grades.begin() + index);
    }

    return grades;
}

student_courses suggested_courses(student_courses allowed_courses, float average_grade) {

    student_courses courses;
    int max_unit = 0;
    if (average_grade >= HIGH_GRADE) max_unit = MAX_UNIT_FOR_UP_17;
    else max_unit = MAX_UNIT_FOR_LESS_17;
    int units = 0;

    for (int index = 0; index < allowed_courses.size(); index++) {

        units += allowed_courses[index].units;
        course course_info;
        bool interference = check_interference(index, allowed_courses);

        if (interference && (units <= max_unit)) {
            course_info.units = allowed_courses[index].units;
            course_info.id = allowed_courses[index].id;
            course_info.name = allowed_courses[index].name;
            course_info.schedule = allowed_courses[index].schedule;
            course_info.prerequisites = allowed_courses[index].prerequisites;
            courses.push_back(course_info);
        } else units -= allowed_courses[index].units;
    }

    return courses;
}

bool check_interference(int index, student_courses courses) {

    for (int iterator = 0; iterator < index; ++iterator) {
        for (int i = 0; i < courses[index].schedule.size(); ++i) {
            for (int j = 0; j < courses[iterator].schedule.size(); ++j) {

                int current_course_start_time = time_convert(courses[index].schedule[i], start_time);
                int current_course_end_time = time_convert(courses[index].schedule[i], end_time);
                int check_course_start_time = time_convert(courses[iterator].schedule[j], start_time);
                int check_course_end_time = time_convert(courses[iterator].schedule[j], end_time);

                if (courses[index].schedule[i].day_of_week == courses[iterator].schedule[j].day_of_week) {

                    if (((current_course_start_time < check_course_start_time)
                         && (current_course_end_time > check_course_start_time))
                        || ((current_course_start_time < check_course_end_time)
                            && (current_course_end_time > check_course_end_time))
                        || ((check_course_start_time < current_course_start_time)
                            && (check_course_end_time > current_course_start_time))
                        || ((check_course_start_time < current_course_end_time)
                            && (check_course_end_time > current_course_end_time))
                        || ((current_course_start_time == check_course_start_time)
                            && (current_course_end_time == check_course_end_time)))
                        return false;
                }
            }
        }
    }

    return true;
}

int time_convert(course_time class_time, time_mode mode) {

    int time_number = 0;

    switch (mode) {

        case start_time:
            time_number = class_time.start_minute;
            time_number += (TIME_CONVERTER) * class_time.start_hour;
            break;
        case end_time:
            time_number = class_time.end_minute;
            time_number += (TIME_CONVERTER) * class_time.end_hour;
            break;
    }

    return time_number;
}