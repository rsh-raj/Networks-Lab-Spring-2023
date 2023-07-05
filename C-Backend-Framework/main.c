#include "CBack.h"


// #define IN Request *, int
// #define OUT char *

int user_loader(Request *req, char *pk, int socket, int status)
{   
    // user can do anything with the status code
    // redirect or display forbidden as per need

    // one can use the pk to get the user from the db
    // and then load the user info into the current_user dictinary in request struct

    // populates current user in the req array
    if (!pk || strcmp(pk, "adityachoudhary.01m@gmail.com") != 0)
    {
        char url_f[100];
        strcpy(url_f, "/login");
        strcat(url_f, "?redirect=");
        strcat(url_f, req->url);
        redirect(socket, url_f, NULL);

        return 302;
    }

    else
    {
        // update the current_user dictionary
        return 200;
    }

    // else{
    //     redirect(socket, "/login", NULL);
    //     return 302;
    // }
}

// for now these functions return the pay load to be sent
OUT home(Request *req, int new_socket)
{
    return render_template(new_socket, "index.html");
    // return NULL;
}

OUT about(Request *req, int new_socket)
{
    // Dictionary d = get_json(req);
    // print_dict(d);

    // Dictionary c[4];
    // int val = 20;
    // Dictionary a1 = init_dict();
    // a1.insert(&a1, "name", "aditya", 1);
    // a1.insert(&a1, "age", &val, 0);
    // a1.insert(&a1, "god", "true", 0);

    // Dictionary a2 = init_dict();
    // a1.insert(&a2, "name", "aditya", 1);
    // a1.insert(&a2, "age", &val, 0);
    // a1.insert(&a2, "god", "true", 0);

    // Dictionary a3 = init_dict();
    // a1.insert(&a3, "name", "aditya", 1);
    // a1.insert(&a3, "age", &val, 0);
    // a1.insert(&a3, "god", "true", 0);

    // c[0] = a1;
    // c[1] = a2;
    // c[2] = a3;
    // c[3] = d;

    return render_template(new_socket, "about.html");
    // jsonify(new_socket, 200, c,1,4);

    // return NULL;
}

OUT login(Request *req, int new_socket)
{
    if (strcmp(req->request_method, "POST") == 0)
    {
        // form submitted
        Dictionary d = get_form_data(req);
        void *email = d.search(&d, "email");
        void *pass = d.search(&d, "password");

        int x = strcmp((char *)email, "adityachoudhary.01m@gmail.com");
        int y = strcmp((char *)pass, "1234");


        if (x == 0 && y == 0)
        {
            void *pt = req->query_params.search(&req->query_params, "redirect");
            if (!pt){
                pt = (char *)("/about");
            }
            redirect(new_socket, (char *)(pt), (char *)email);
            // render_template(new_socket, "login.html");
            return NULL;
        }
        else
        {
            // flash(new_socket, "Wow");
            redirect(new_socket, "/login", NULL);
            return NULL;
        }
    }

    return render_template(new_socket, "login.html");
    // redirect(new_socket, "https://www.google.com", NULL);
    // return NULL;
}

OUT quiz(Request *req, int new_socket)
{
    // return render_template(new_socket, "quiz.html");
    // return NULL;
    Response *res = new_response();
    res->status_code = 200;
    set_status_message(res, "OK");
    set_header(res, "AUTH_TOKEN", "dfjiuydyfguhu");
    set_header(res, "Content-Type", "text/html");
    set_body(res, "This is the C backend Library!!");

    send_response(res, new_socket);
    return NULL;
}

OUT gallery(Request *req, int new_socket)
{
    // return render_template(new_socket, "gallery.html");
    return "This is gallery!";
    // redirect(new_socket, "/quiz", NULL);
    // return NULL;
}

OUT about_id(Request *req, int new_socket)
{   
    
    Dictionary d = req->query_params;

    void *val = d.search(&d, "id");
    Dictionary c = init_dict();
    c.insert(&c, "id", val, 0);

    return jsonify(new_socket, 200, &c, 0, 1);

    // return NULL;
}

int main(int argc, char *argv[])
{
    int port = 80;
    if (argc > 1)
    {
        port = atoi(argv[1]);
    }

    char *methods[] = {"GET", "POST"};
    int num = 2;
    LoginManager();
    add_route("/", &home, methods, num);
    add_route("/login", &login, methods, num);
    add_route("/about", &about, methods, num);
    add_route("/quiz", &quiz, methods, num);
    add_route("/gallery", &gallery, methods, num);
    add_route("/gallery/<int:id>", &about_id, methods, num);
    CORS_enable("http://127.0.0.1:8080, http://127.0.0.1:8081");
    login_required("/about");
    login_required("/quiz");
    login_required("/gallery");
    create_app(port);

    return 0;
}