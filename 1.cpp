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
#define FIRST_INDEX 0

using namespace std;

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
void print_courses_id(student_courses courses);

int main(int argc, char *argv[]) {

    arguments files_path = get_arguments(argv);
    student_grades grades = read_grades_data(files_path.grades);
    student_courses courses = read_course_data(files_path.courses);
    student_courses all_allowed_courses = find_allowed_courses(courses, grades);
    sort(all_allowed_courses.begin(), all_allowed_courses.end(), sort_course_by_name);
    print_courses_id(all_allowed_courses);

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

bool sort_course_by_name(course course1, course course2) {

    return course2.name > course1.name;
}

void print_courses_id(student_courses courses) {

    for (int index = 0; index < courses.size(); ++index) {
        cout << courses[index].id << endl;
    }
}