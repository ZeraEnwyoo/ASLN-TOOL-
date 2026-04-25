#ifndef ASLN_AGENCY_DATA_H
#define ASLN_AGENCY_DATA_H

typedef struct {
    const char* name;
    const char* display_name;
    const char* locations[5];
    int location_count;
    int always_available;
} agency_info_t;

static agency_info_t agencies[] = {
    {.name="nasa", .display_name="NASA", .locations={"The Moon","The Sun","Mars","Saturn","Jupiter"}, .location_count=5, .always_available=1},
    {.name="spacex", .display_name="SpaceX", .locations={"Starlink Satellite","Dragon Capsule","Starship","Falcon Heavy","Tesla Roadster (2018)"}, .location_count=5, .always_available=1},
    {.name="esa", .display_name="ESA", .locations={"ESA Satellite Network","Columbus Module","Kourou Spaceport","ESTEC","ESOC"}, .location_count=5, .always_available=1},
    {.name="cnsa", .display_name="CNSA", .locations={"Tiangong Space Station","Wenchang","Jiuquan"}, .location_count=3, .always_available=0},
    {.name="roscosmos", .display_name="Roscosmos", .locations={"Baikonur Cosmodrome","Vostochny","ISS Russian Segment"}, .location_count=3, .always_available=0}
};
static const int agency_count = 5;

#endif
