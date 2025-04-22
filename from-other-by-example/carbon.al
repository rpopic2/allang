// https://youtu.be/omrY53kbVoA

NewsArticle:
    make: (str headline, str body_html =>NewsArticle)

    as_html: (#self me =>str)

    publish: (addr #self me)
        std.datetime.now=> =[me.published]

    struct {
        str headline,
        str body_html,
        datetime? published,
    }

    make_publish_and_print: (str headline, str body_html)
        article :: NewsArticle.make=> =[]
        article publish=>
        [article] ashtml=> std.print=>


UIWidget:
    draw: (@self, Screen s)
    click: (addr @self, i32 x, i32 y)

Button:

    draw: (@self, Screen s)
        // ...
    click: (@self, i32 x, i32 y)
        // ...
    move_to: (@self, i32 x, i32 y)
        // ...

    struct vtable {
        addr (@self, i32 x, i32 y) move_to
    }
    struct {
        |addr vtable| vtable
    }

ImageButton:
    struct Button { }


Summary:
    summarize: (@self =>str)

@generic print_summary.T: (Summary T, T x)
    x Summarize=> print=>


NewsArticle:
    summarize: (@self =>str)

summarize_news: (NewsArticle n =>str)
    n print_summary=>
    n summarize=>

0 =[x

article.published] ? "published" : "not published"
print=>

