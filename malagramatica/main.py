# Don't waste your time reading this it is just AI trash
# @LLM-ASSISTED
import gi
import os
gi.require_version("Gtk", "4.0")
gi.require_version("GtkSource", "5")
from gi.repository import Gtk, GtkSource, Gdk, Pango

from langchain_multi_provider import MultiProviderLLM

class SpellCheckApp(Gtk.Application):
    def __init__(self):
        super().__init__(application_id='com.ejemplo.SpellCheckApp')
        self.connect("activate", self.on_activate)
        self.suggestions_map = {}

        api_key = os.getenv("MIGRAMATICA_LLM_API_KEY", "")
        provider = os.getenv("MIGRAMATICA_LLM_PROVIDER", "openai")
        self.llm = MultiProviderLLM(provider=provider, api_key=api_key)

    def on_activate(self, app):
        window = Gtk.ApplicationWindow(application=app)
        window.set_title("Revisor de Texto")
        window.set_default_size(600, 400)

        self.buffer = GtkSource.Buffer()
        self.view = GtkSource.View.new_with_buffer(self.buffer)
        self.view.set_wrap_mode(Gtk.WrapMode.WORD)
        self.view.set_monospace(True)
        self.view.set_hexpand(True)
        self.view.set_vexpand(True)
        self.view.set_editable(False)

        self.tag_wrong = self.buffer.create_tag(
            "wrong",
            underline=Pango.Underline.ERROR,
            foreground="red",
            background="#ffe6e6"
        )

        self.view.set_has_tooltip(True)
        self.view.connect("query-tooltip", self.on_query_tooltip)

        self.entry = Gtk.Entry()
        self.entry.set_placeholder_text("Introduce el texto y presiona Enter")
        self.entry.connect("activate", self.on_text_entered)

        box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=6)
        box.append(self.entry)
        scrolled = Gtk.ScrolledWindow()
        scrolled.set_child(self.view)
        box.append(scrolled)

        window.set_child(box)
        window.show()

    def on_text_entered(self, entry):
        texto = entry.get_text()
        self.buffer.set_text(texto)
        self.suggestions_map.clear()
        self.buffer.remove_all_tags(self.buffer.get_start_iter(), self.buffer.get_end_iter())

        try:
            resultado = self.llm.run("prompt.txt", texto, language="english")
        except Exception as e:
            print(f"Error llamando al LLM: {e}")
            resultado = {"wrong": []}

        for item in resultado.get("wrong", []):
            start, end = item["position"]
            iter_start = self.buffer.get_iter_at_offset(start)
            iter_end = self.buffer.get_iter_at_offset(end)
            self.buffer.apply_tag(self.tag_wrong, iter_start, iter_end)
            self.suggestions_map[(start, end)] = item.get("suggestions", [])

    def on_query_tooltip(self, widget, x, y, keyboard_mode, tooltip):
        x_buf, y_buf = widget.window_to_buffer_coords(Gtk.TextWindowType.WIDGET, x, y)
        is_text, text_iter = self.view.get_iter_at_location(x_buf, y_buf)
        if not is_text or not text_iter:
            return False

        offset = text_iter.get_offset()

        for (start, end), suggestions in self.suggestions_map.items():
            if start <= offset < end:
                tooltip.set_markup("<b>Sugerencias:</b> " + ", ".join(suggestions))
                return True

        return False

if __name__ == "__main__":
    app = SpellCheckApp()
    app.run()
