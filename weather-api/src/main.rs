use actix::{Actor, StreamHandler};
use actix_web::{get, post, web, App, HttpResponse, HttpRequest, Error, HttpServer, Responder};
use actix_web_actors::ws;

struct Socket;

impl Actor for Socket {
    type Context = ws::WebsocketContext<Self>;
}

impl StreamHandler<Result<ws::Message, ws::ProtocolError>> for Socket {
    fn handle(&mut self, msg: Result<ws::Message, ws::ProtocolError>, ctx: &mut Self::Context) {
        match msg {
            Ok(ws::Message::Ping(msg)) => {
                ctx.pong(&msg);
                println!("pinged");
            },
            Ok(ws::Message::Text(text)) => {
                ctx.text(&text);
                println!("{}", &text);
            },
            Ok(ws::Message::Binary(bin)) => ctx.binary(bin),
            _ => (),
        }
    }
}

//#[get("/")]
//async fn hello() -> impl Responder {
//    println!("Hello, Socialify Weather!");
//    HttpResponse::Ok().body("Hello, Socialify Weather!")
//}

async fn index(req: HttpRequest, stream: web::Payload) -> Result<HttpResponse, Error> {
    let resp = ws::start(Socket {}, &req, stream);
    println!("{:?}", resp);
    resp
}

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    HttpServer::new(|| App::new().route("/receiveWsData/", web::get().to(index)))
        .bind("192.168.8.199:83")?
        .run()
        .await
}
