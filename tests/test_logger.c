#include "vasq/logger.h"

int main() {
    int ret;
    unsigned char short_data[10];
    const char data[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed non tellus vel turpis mattis porta lobortis at mi. Curabitur sed diam est. Praesent eget ex nulla. Morbi fermentum, eros aliquam ornare tempor, tellus metus dignissim lacus, in auctor nisi sem vitae magna. Sed eu lacus sed libero scelerisque consectetur. Nam vel quam condimentum, aliquet nunc at, lobortis diam. Curabitur eget sodales diam. Fusce ut varius enim. Sed at urna quam. Etiam dapibus dui tortor, ac efficitur libero venenatis suscipit. Duis risus purus, pretium in malesuada sed, lobortis eget leo. Nulla facilisi. Suspendisse venenatis erat a justo ornare rutrum. Phasellus sed risus quis neque suscipit efficitur.  Sed vehicula vitae tellus vel pulvinar. Aenean metus ante, vehicula vel consequat sit amet, laoreet nec ipsum. Etiam sed nulla velit. Aenean volutpat vitae enim eget feugiat. Aliquam ac arcu in ligula malesuada facilisis. Proin consectetur sem arcu, eget dictum diam gravida ut. Proin ultrices bibendum turpis, et consequat dui.  Vestibulum ultrices ultricies porttitor. Proin a ullamcorper libero, et efficitur orci. Nunc viverra interdum erat sit amet finibus. Duis lacus mauris, rhoncus sit amet euismod et, vulputate at eros. Quisque maximus turpis et quam placerat, eu semper magna efficitur. Donec nec vulputate justo, sed eleifend nunc. Donec dictum mattis massa, a feugiat elit pretium nec.  Maecenas eleifend enim in quam cursus, a sagittis neque tempus. Maecenas eu massa id dolor sollicitudin fermentum at in nunc. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Nulla at bibendum ex. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Fusce condimentum, ex eu hendrerit hendrerit, turpis dui scelerisque risus, nec sodales urna sem eget felis. Morbi elit libero, pulvinar sed euismod vitae, euismod quis elit. Mauris vehicula mi mollis tortor eleifend pretium. Etiam egestas congue metus, et feugiat ex rhoncus ac.  Maecenas sed magna lorem. Nam enim ex, porttitor a elit nec, commodo tempor nisi. Nulla tristique et odio in consectetur. Fusce accumsan ante arcu. Mauris euismod arcu sit amet libero porttitor vestibulum. Sed imperdiet justo nibh, sed viverra erat sagittis et. Mauris vel malesuada odio.";

    ret = VASQ_LOG_INIT(VASQ_LL_DEBUG, STDOUT_FILENO, true);
    if ( ret != VASQ_RET_OK ) {
        return ret;
    }

    VASQ_ALWAYS("This is an ALWAYS message");
    VASQ_CRITICAL("This is a CRITICAL message");
    VASQ_ERROR("This is an ERROR message");
    VASQ_WARNING("This is a WARNING message");
    VASQ_INFO("This is an INFO message");
    VASQ_DEBUG("This is a DEBUG message");

    for (unsigned int k=0; k<sizeof(short_data); k++) {
        short_data[k] = 10*k;
    }
    VASQ_HEXDUMP("Short data", short_data, sizeof(short_data));

    VASQ_HEXDUMP("Lorem ipsum", data, sizeof(data));

    return 0;
}
